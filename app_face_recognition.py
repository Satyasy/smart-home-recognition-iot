from flask import Flask, request, jsonify
from flask_cors import CORS
import face_recognition
import numpy as np
import cv2
import firebase_admin
from firebase_admin import credentials, db
import base64
from datetime import datetime
import os
import tempfile
import shutil

app = Flask(__name__)
CORS(app)  # Enable CORS untuk semua routes

# Initialize Firebase
cred = credentials.Certificate('serviceAccountKey.json')
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://iot-rc-ef82d-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

# Reference ke Firebase RTDB
users_ref = db.reference('users')
logs_ref = db.reference('access_logs')

# Threshold untuk face recognition (0.6 is default, lower = more strict)
TOLERANCE = 0.6

# Temporary directory untuk menyimpan images
TEMP_DIR = tempfile.mkdtemp()

def decode_image(base64_string):
    """Decode base64 image dari ESP32-CAM"""
    try:
        img_data = base64.b64decode(base64_string)
        nparr = np.frombuffer(img_data, np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        return img
    except Exception as e:
        print(f"Error decoding image: {e}")
        return None

def get_face_encoding(image):
    """Extract face encoding menggunakan face_recognition"""
    try:
        # Convert BGR to RGB (OpenCV uses BGR, face_recognition uses RGB)
        rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        
        # Find face locations
        face_locations = face_recognition.face_locations(rgb_image, model="hog")
        
        if not face_locations:
            return None, "No face detected"
        
        if len(face_locations) > 1:
            return None, "Multiple faces detected. Please ensure only one face is visible"
        
        # Get face encoding
        face_encodings = face_recognition.face_encodings(rgb_image, face_locations)
        
        if not face_encodings:
            return None, "Could not encode face"
        
        return face_encodings[0].tolist(), None
        
    except Exception as e:
        return None, f"Error extracting face: {str(e)}"

def load_known_faces():
    """Load semua face encodings dari Firebase"""
    users = users_ref.get()
    known_faces = {}
    
    if users:
        for user_id, user_data in users.items():
            if 'face_encoding' in user_data and user_data.get('status') == 'active':
                encoding = user_data['face_encoding']
                known_faces[user_id] = {
                    'encoding': np.array(encoding),
                    'name': user_data.get('name', 'Unknown'),
                    'email': user_data.get('email', ''),
                    'phone': user_data.get('phone', '')
                }
    
    return known_faces

@app.route('/api/register', methods=['POST'])
def register_face():
    """
    Register wajah baru
    Body: {
        "image": "base64_encoded_image",
        "name": "Nama User",
        "email": "email@example.com",
        "phone": "08123456789" (optional)
    }
    """
    try:
        data = request.json
        
        if not data or 'image' not in data or 'name' not in data:
            return jsonify({
                'success': False,
                'message': 'Missing required fields: image and name'
            }), 400
        
        # Decode image
        image = decode_image(data['image'])
        if image is None:
            return jsonify({
                'success': False,
                'message': 'Invalid image format'
            }), 400
        
        # Get face encoding
        face_encoding, error = get_face_encoding(image)
        if error:
            return jsonify({
                'success': False,
                'message': error
            }), 400
        
        # Check if face already registered
        known_faces = load_known_faces()
        for user_id, user_info in known_faces.items():
            matches = face_recognition.compare_faces(
                [user_info['encoding']], 
                np.array(face_encoding),
                tolerance=TOLERANCE
            )
            
            if matches[0]:
                distance = face_recognition.face_distance([user_info['encoding']], np.array(face_encoding))[0]
                return jsonify({
                    'success': False,
                    'message': f'Face already registered as {user_info["name"]} (distance: {round(distance, 4)})'
                }), 400
        
        # Generate user ID
        user_id = data.get('user_id', f"user_{datetime.now().strftime('%Y%m%d%H%M%S')}")
        
        # Save to Firebase
        user_data = {
            'name': data['name'],
            'email': data.get('email', ''),
            'phone': data.get('phone', ''),
            'face_encoding': face_encoding,
            'registered_at': datetime.now().isoformat(),
            'status': 'active',
            'model': 'face_recognition'
        }
        
        users_ref.child(user_id).set(user_data)
        
        return jsonify({
            'success': True,
            'message': f'User {data["name"]} registered successfully',
            'user_id': user_id
        }), 201
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/recognize', methods=['POST'])
def recognize_face():
    """
    Recognize wajah dari ESP32-CAM
    Body: {
        "image": "base64_encoded_image"
    }
    """
    try:
        data = request.json
        
        if not data or 'image' not in data:
            return jsonify({
                'success': False,
                'authorized': False,
                'message': 'No image provided'
            }), 400
        
        # Decode image
        image = decode_image(data['image'])
        if image is None:
            return jsonify({
                'success': False,
                'authorized': False,
                'message': 'Invalid image format'
            }), 400
        
        # Get face encoding
        face_encoding, error = get_face_encoding(image)
        if error:
            # Log failed attempt
            log_data = {
                'timestamp': datetime.now().isoformat(),
                'authorized': False,
                'user_id': 'unknown',
                'user_name': 'Unknown',
                'confidence': 0,
                'reason': error
            }
            logs_ref.push(log_data)
            
            return jsonify({
                'success': False,
                'authorized': False,
                'message': error
            }), 200
        
        # Load known faces
        known_faces = load_known_faces()
        
        if not known_faces:
            return jsonify({
                'success': False,
                'authorized': False,
                'message': 'No registered users in database'
            }), 200
        
        # Compare with known faces
        best_match = None
        best_distance = float('inf')
        
        face_encoding_array = np.array(face_encoding)
        
        for user_id, user_info in known_faces.items():
            matches = face_recognition.compare_faces(
                [user_info['encoding']], 
                face_encoding_array,
                tolerance=TOLERANCE
            )
            
            if matches[0]:
                distance = face_recognition.face_distance([user_info['encoding']], face_encoding_array)[0]
                
                if distance < best_distance:
                    best_distance = distance
                    confidence = round((1 - distance) * 100, 2)
                    
                    # Filter: Hanya terima akurasi >= 70%
                    if confidence >= 70.0:
                        best_match = {
                            'user_id': user_id,
                            'name': user_info['name'],
                            'email': user_info['email'],
                            'phone': user_info['phone'],
                            'confidence': confidence,
                            'distance': round(float(distance), 4)
                        }
        
        # Log access attempt
        log_data = {
            'timestamp': datetime.now().isoformat(),
            'authorized': best_match is not None,
            'user_id': best_match['user_id'] if best_match else 'unknown',
            'user_name': best_match['name'] if best_match else 'Unknown',
            'confidence': best_match['confidence'] if best_match else 0
        }
        logs_ref.push(log_data)
        
        if best_match:
            return jsonify({
                'success': True,
                'authorized': True,
                'user': best_match,
                'message': f'Welcome {best_match["name"]}!'
            }), 200
        else:
            return jsonify({
                'success': True,
                'authorized': False,
                'message': 'Face not recognized or confidence < 70%'
            }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'authorized': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/verify', methods=['POST'])
def verify_faces():
    """
    Verify apakah dua wajah adalah orang yang sama
    Body: {
        "image1": "base64_encoded_image",
        "image2": "base64_encoded_image"
    }
    """
    try:
        data = request.json
        
        if not data or 'image1' not in data or 'image2' not in data:
            return jsonify({
                'success': False,
                'message': 'Missing required fields: image1 and image2'
            }), 400
        
        # Decode images
        image1 = decode_image(data['image1'])
        image2 = decode_image(data['image2'])
        
        if image1 is None or image2 is None:
            return jsonify({
                'success': False,
                'message': 'Invalid image format'
            }), 400
        
        # Get face encodings
        encoding1, error1 = get_face_encoding(image1)
        encoding2, error2 = get_face_encoding(image2)
        
        if error1 or error2:
            return jsonify({
                'success': False,
                'message': error1 or error2
            }), 400
        
        # Compare faces
        matches = face_recognition.compare_faces(
            [np.array(encoding1)], 
            np.array(encoding2),
            tolerance=TOLERANCE
        )
        distance = face_recognition.face_distance([np.array(encoding1)], np.array(encoding2))[0]
        
        return jsonify({
            'success': True,
            'verified': bool(matches[0]),
            'distance': round(float(distance), 4),
            'threshold': TOLERANCE,
            'model': 'face_recognition',
            'similarity': round((1 - distance) * 100, 2)
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Verification error: {str(e)}'
        }), 500

@app.route('/api/users', methods=['GET'])
def get_users():
    """Get semua registered users"""
    try:
        users = users_ref.get()
        
        if not users:
            return jsonify({
                'success': True,
                'users': [],
                'count': 0
            }), 200
        
        users_list = []
        for user_id, user_data in users.items():
            users_list.append({
                'user_id': user_id,
                'name': user_data.get('name', ''),
                'email': user_data.get('email', ''),
                'phone': user_data.get('phone', ''),
                'registered_at': user_data.get('registered_at', ''),
                'status': user_data.get('status', 'active'),
                'model': user_data.get('model', 'unknown')
            })
        
        return jsonify({
            'success': True,
            'users': users_list,
            'count': len(users_list)
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/user/<user_id>', methods=['GET'])
def get_user(user_id):
    """Get detail user tertentu"""
    try:
        user = users_ref.child(user_id).get()
        
        if not user:
            return jsonify({
                'success': False,
                'message': 'User not found'
            }), 404
        
        user_data = {
            'user_id': user_id,
            'name': user.get('name', ''),
            'email': user.get('email', ''),
            'phone': user.get('phone', ''),
            'registered_at': user.get('registered_at', ''),
            'status': user.get('status', 'active'),
            'model': user.get('model', 'unknown')
        }
        
        return jsonify({
            'success': True,
            'user': user_data
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/user/<user_id>', methods=['PUT'])
def update_user(user_id):
    """Update user data (tanpa face encoding)"""
    try:
        user = users_ref.child(user_id).get()
        
        if not user:
            return jsonify({
                'success': False,
                'message': 'User not found'
            }), 404
        
        data = request.json
        update_data = {}
        
        if 'name' in data:
            update_data['name'] = data['name']
        if 'email' in data:
            update_data['email'] = data['email']
        if 'phone' in data:
            update_data['phone'] = data['phone']
        if 'status' in data:
            update_data['status'] = data['status']
        
        if not update_data:
            return jsonify({
                'success': False,
                'message': 'No data to update'
            }), 400
        
        users_ref.child(user_id).update(update_data)
        
        return jsonify({
            'success': True,
            'message': f'User {user_id} updated successfully'
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/user/<user_id>', methods=['DELETE'])
def delete_user(user_id):
    """Delete user dari database"""
    try:
        user = users_ref.child(user_id).get()
        
        if not user:
            return jsonify({
                'success': False,
                'message': 'User not found'
            }), 404
        
        users_ref.child(user_id).delete()
        
        return jsonify({
            'success': True,
            'message': f'User {user.get("name", user_id)} deleted successfully'
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/logs', methods=['GET'])
def get_logs():
    """Get access logs"""
    try:
        limit = request.args.get('limit', 50, type=int)
        logs = logs_ref.order_by_child('timestamp').limit_to_last(limit).get()
        
        if not logs:
            return jsonify({
                'success': True,
                'logs': [],
                'count': 0
            }), 200
        
        logs_list = []
        for log_id, log_data in logs.items():
            logs_list.append({
                'log_id': log_id,
                **log_data
            })
        
        # Sort by timestamp descending
        logs_list.sort(key=lambda x: x['timestamp'], reverse=True)
        
        return jsonify({
            'success': True,
            'logs': logs_list,
            'count': len(logs_list)
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/logs/clear', methods=['DELETE'])
def clear_logs():
    """Clear all access logs"""
    try:
        logs_ref.delete()
        
        return jsonify({
            'success': True,
            'message': 'All logs cleared successfully'
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Server error: {str(e)}'
        }), 500

@app.route('/api/config', methods=['GET'])
def get_config():
    """Get current configuration"""
    return jsonify({
        'success': True,
        'config': {
            'model': 'face_recognition (dlib)',
            'tolerance': TOLERANCE
        }
    }), 200

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'model': 'face_recognition',
        'version': '2.0.1'
    }), 200

# Cleanup temporary directory on shutdown
import atexit

@atexit.register
def cleanup():
    """Cleanup temporary directory"""
    if os.path.exists(TEMP_DIR):
        shutil.rmtree(TEMP_DIR)

if __name__ == '__main__':
    print(f"""
    ╔════════════════════════════════════════════════╗
    ║  Smart Home Face Recognition Server            ║
    ╠════════════════════════════════════════════════╣
    ║  Model: face_recognition (dlib)                ║
    ║  Tolerance: {TOLERANCE:<36} ║
    ║  Server: http://0.0.0.0:5000                   ║
    ╚════════════════════════════════════════════════╝
    """)
    app.run(host='0.0.0.0', port=5000, debug=True)
