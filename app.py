from flask import Flask, request, jsonify
from deepface import DeepFace
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

# Initialize Firebase
cred = credentials.Certificate('serviceAccountKey.json')  # Download dari Firebase Console
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://iot-rc-ef82d-default-rtdb.asia-southeast1.firebasedatabase.app/'  # Ganti dengan URL RTDB Anda
})

# Reference ke Firebase RTDB
users_ref = db.reference('users')
logs_ref = db.reference('access_logs')

# DeepFace configuration
MODEL_NAME = "Facenet512"  # Options: VGG-Face, Facenet, Facenet512, ArcFace, Dlib, SFace
DISTANCE_METRIC = "cosine"  # Options: cosine, euclidean, euclidean_l2
DETECTOR_BACKEND = "opencv"  # Options: opencv, ssd, dlib, mtcnn, retinaface

# Threshold untuk face recognition (semakin kecil semakin strict)
# Facenet512 cosine: 0.30, Facenet: 0.40, VGG-Face: 0.40, ArcFace: 0.68
THRESHOLD = 0.40

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

def save_temp_image(image, prefix="temp"):
    """Save image to temporary file"""
    temp_path = os.path.join(TEMP_DIR, f"{prefix}_{datetime.now().strftime('%Y%m%d%H%M%S%f')}.jpg")
    cv2.imwrite(temp_path, image)
    return temp_path

def get_face_embedding(image_path):
    """Extract face embedding menggunakan DeepFace"""
    try:
        # Detect face dan extract embedding
        embedding_objs = DeepFace.represent(
            img_path=image_path,
            model_name=MODEL_NAME,
            detector_backend=DETECTOR_BACKEND,
            enforce_detection=True
        )
        
        if not embedding_objs:
            return None, "No face detected"
        
        if len(embedding_objs) > 1:
            return None, "Multiple faces detected. Please ensure only one face is visible"
        
        embedding = embedding_objs[0]["embedding"]
        return embedding, None
        
    except ValueError as e:
        if "Face could not be detected" in str(e):
            return None, "No face detected in image"
        return None, str(e)
    except Exception as e:
        return None, f"Error extracting face: {str(e)}"

def calculate_distance(embedding1, embedding2):
    """Calculate distance between two embeddings"""
    embedding1 = np.array(embedding1)
    embedding2 = np.array(embedding2)
    
    if DISTANCE_METRIC == "cosine":
        # Cosine distance
        dot_product = np.dot(embedding1, embedding2)
        norm1 = np.linalg.norm(embedding1)
        norm2 = np.linalg.norm(embedding2)
        distance = 1 - (dot_product / (norm1 * norm2))
    elif DISTANCE_METRIC == "euclidean":
        # Euclidean distance
        distance = np.linalg.norm(embedding1 - embedding2)
    elif DISTANCE_METRIC == "euclidean_l2":
        # Euclidean L2 distance
        distance = np.sqrt(np.sum((embedding1 - embedding2) ** 2))
    else:
        distance = np.linalg.norm(embedding1 - embedding2)
    
    return distance

def load_known_faces():
    """Load semua face embeddings dari Firebase"""
    users = users_ref.get()
    known_faces = {}
    
    if users:
        for user_id, user_data in users.items():
            if 'face_embedding' in user_data and user_data.get('status') == 'active':
                embedding = user_data['face_embedding']
                known_faces[user_id] = {
                    'embedding': embedding,
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
    temp_image_path = None
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
        
        # Save to temporary file
        temp_image_path = save_temp_image(image, "register")
        
        # Get face embedding
        face_embedding, error = get_face_embedding(temp_image_path)
        if error:
            return jsonify({
                'success': False,
                'message': error
            }), 400
        
        # Check if face already registered
        known_faces = load_known_faces()
        for user_id, user_info in known_faces.items():
            distance = calculate_distance(user_info['embedding'], face_embedding)
            
            if distance < THRESHOLD:
                return jsonify({
                    'success': False,
                    'message': f'Face already registered as {user_info["name"]} (similarity: {round((1-distance)*100, 2)}%)'
                }), 400
        
        # Generate user ID
        user_id = data.get('user_id', f"user_{datetime.now().strftime('%Y%m%d%H%M%S')}")
        
        # Save to Firebase
        user_data = {
            'name': data['name'],
            'email': data.get('email', ''),
            'phone': data.get('phone', ''),
            'face_embedding': face_embedding,
            'registered_at': datetime.now().isoformat(),
            'status': 'active',
            'model': MODEL_NAME
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
    finally:
        # Cleanup temporary file
        if temp_image_path and os.path.exists(temp_image_path):
            os.remove(temp_image_path)

@app.route('/api/recognize', methods=['POST'])
def recognize_face():
    """
    Recognize wajah dari ESP32-CAM
    Accepts:
    - Content-Type: application/json with {"image": "base64_string"}
    - Content-Type: image/jpeg (raw JPEG data dari ESP32-CAM)
    
    Response: {
        "success": true/false,
        "recognized": true/false,
        "name": "...",
        "confidence": 0.85,
        "message": "..."
    }
    """
    temp_image_path = None
    try:
        # Check if raw JPEG image (dari ESP32-CAM)
        if request.content_type and 'image/jpeg' in request.content_type:
            # Raw JPEG data
            jpeg_data = request.data
            nparr = np.frombuffer(jpeg_data, np.uint8)
            image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            
            if image is None:
                return jsonify({
                    'success': False,
                    'recognized': False,
                    'message': 'Invalid JPEG image'
                }), 400
        else:
            # JSON dengan base64
            data = request.json
            
            if not data or 'image' not in data:
                return jsonify({
                    'success': False,
                    'recognized': False,
                    'message': 'No image provided'
                }), 400
            
            # Decode image
            image = decode_image(data['image'])
            if image is None:
                return jsonify({
                    'success': False,
                    'recognized': False,
                    'message': 'Invalid image format'
                }), 400
        
        # Save to temporary file
        temp_image_path = save_temp_image(image, "recognize")
        
        # Get face embedding
        face_embedding, error = get_face_embedding(temp_image_path)
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
        
        for user_id, user_info in known_faces.items():
            distance = calculate_distance(user_info['embedding'], face_embedding)
            
            if distance < THRESHOLD and distance < best_distance:
                best_distance = distance
                confidence = round((1 - distance) * 100, 2)
                best_match = {
                    'user_id': user_id,
                    'name': user_info['name'],
                    'email': user_info['email'],
                    'phone': user_info['phone'],
                    'confidence': confidence,
                    'distance': round(distance, 4)
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
                'recognized': True,
                'authorized': True,
                'name': best_match['name'],
                'confidence': best_match['confidence'] / 100,  # Return as 0.0-1.0 untuk ESP32-CAM
                'user': best_match,
                'message': f'Welcome {best_match["name"]}!'
            }), 200
        else:
            return jsonify({
                'success': True,
                'recognized': False,
                'authorized': False,
                'message': 'Face not recognized or confidence too low'
            }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'authorized': False,
            'message': f'Server error: {str(e)}'
        }), 500
    finally:
        # Cleanup temporary file
        if temp_image_path and os.path.exists(temp_image_path):
            os.remove(temp_image_path)

@app.route('/api/verify', methods=['POST'])
def verify_faces():
    """
    Verify apakah dua wajah adalah orang yang sama
    Body: {
        "image1": "base64_encoded_image",
        "image2": "base64_encoded_image"
    }
    """
    temp_path1 = None
    temp_path2 = None
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
        
        # Save to temporary files
        temp_path1 = save_temp_image(image1, "verify1")
        temp_path2 = save_temp_image(image2, "verify2")
        
        # Verify using DeepFace
        result = DeepFace.verify(
            img1_path=temp_path1,
            img2_path=temp_path2,
            model_name=MODEL_NAME,
            distance_metric=DISTANCE_METRIC,
            detector_backend=DETECTOR_BACKEND,
            enforce_detection=True
        )
        
        return jsonify({
            'success': True,
            'verified': result['verified'],
            'distance': result['distance'],
            'threshold': result['threshold'],
            'model': result['model'],
            'similarity': round((1 - result['distance']) * 100, 2)
        }), 200
        
    except Exception as e:
        return jsonify({
            'success': False,
            'message': f'Verification error: {str(e)}'
        }), 500
    finally:
        # Cleanup temporary files
        if temp_path1 and os.path.exists(temp_path1):
            os.remove(temp_path1)
        if temp_path2 and os.path.exists(temp_path2):
            os.remove(temp_path2)

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
    """Update user data (tanpa face embedding)"""
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
            'model': MODEL_NAME,
            'distance_metric': DISTANCE_METRIC,
            'detector_backend': DETECTOR_BACKEND,
            'threshold': THRESHOLD
        }
    }), 200

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'model': MODEL_NAME,
        'version': '2.0.0'
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
    ║  Smart Home Face Recognition Server (DeepFace) ║
    ╠════════════════════════════════════════════════╣
    ║  Model: {MODEL_NAME:<38} ║
    ║  Distance Metric: {DISTANCE_METRIC:<29} ║
    ║  Threshold: {THRESHOLD:<35} ║
    ║  Server: http://0.0.0.0:5000                   ║
    ╚════════════════════════════════════════════════╝
    """)
    app.run(host='0.0.0.0', port=5000, debug=True)