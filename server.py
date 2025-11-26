from flask import Flask, request, jsonify
import face_recognition
import numpy as np
import pickle
import os
import math

app = Flask(__name__)

KNOWN_FACES_FILE = "known_faces.pkl"

# Load known faces (if exists)
if os.path.exists(KNOWN_FACES_FILE):
    with open(KNOWN_FACES_FILE, "rb") as f:
        known_faces = pickle.load(f)
else:
    known_faces = {"names": [], "encodings": []}

def save_known_faces():
    with open(KNOWN_FACES_FILE, "wb") as f:
        pickle.dump(known_faces, f)

# Convert face_distance → confidence % (recommended formula by ageitgey)
def face_distance_to_conf(face_distance, face_match_threshold=0.6):
    if face_distance > face_match_threshold:
        rng = (1.0 - face_match_threshold)
        linear_val = (1.0 - face_distance) / (rng * 2.0)
        return linear_val
    else:
        rng = face_match_threshold
        linear_val = 1.0 - (face_distance / (rng * 2.0))
        return linear_val + ((1.0 - linear_val) * math.pow((linear_val - 0.5) * 2, 0.2))

@app.route("/")
def home():
    return "Face Recognition Server OK", 200

@app.route("/register", methods=["POST"])
def register_face():
    if "name" not in request.form:
        return jsonify({"error": "Name required"}), 400

    name = request.form["name"]

    # Accept file OR raw binary image
    if "image" in request.files:
        file = request.files["image"]
        img = face_recognition.load_image_file(file)
    else:
        img = face_recognition.load_image_file(request.data)

    encodings = face_recognition.face_encodings(img)
    if len(encodings) == 0:
        return jsonify({"error": "No face found"}), 400

    encoding = encodings[0]
    known_faces["names"].append(name)
    known_faces["encodings"].append(encoding)
    save_known_faces()

    return jsonify({"status": "registered", "name": name}), 200

@app.route("/recognize", methods=["POST"])
def recognize_face():
    if "image" in request.files:
        file = request.files["image"]
        img = face_recognition.load_image_file(file)
    else:
        img = face_recognition.load_image_file(request.data)

    # Encode face
    encodings = face_recognition.face_encodings(img)
    if len(encodings) == 0:
        return jsonify({"authorized": False, "reason": "no-face"}), 200

    unknown_encoding = encodings[0]

    distances = face_recognition.face_distance(known_faces["encodings"], unknown_encoding)

    if len(distances) == 0:
        return jsonify({"authorized": False, "reason": "no-registered-faces"}), 200

    best_idx = np.argmin(distances)
    best_distance = distances[best_idx]
    confidence = face_distance_to_conf(best_distance)

    authorized = confidence >= 0.50  # threshold

    return jsonify({
        "authorized": authorized,
        "name": known_faces["names"][best_idx] if authorized else "unknown",
        "confidence": round(confidence * 100, 2)
    }), 200

@app.route("/known_faces")
def list_faces():
    return jsonify({"registered": known_faces["names"]})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
