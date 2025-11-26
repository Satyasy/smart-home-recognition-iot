from flask import Flask, request, jsonify
import numpy as np
import face_recognition

app = Flask(__name__)

# Load embedding user
user_encoding = np.load("user1.npy")   # file embedding

@app.route("/process-face", methods=["POST"])
def process():
    img_bytes = request.data
    
    np_img = np.frombuffer(img_bytes, np.uint8)
    img = face_recognition.load_image_file(np_img)

    face_locations = face_recognition.face_locations(img)
    if len(face_locations) == 0:
        return jsonify({"match": False, "reason": "no face"})

    encodings = face_recognition.face_encodings(img, face_locations)
    if len(encodings) == 0:
        return jsonify({"match": False, "reason": "no encoding"})

    face = encodings[0]
    distance = face_recognition.face_distance([user_encoding], face)[0]
    similarity = 1 - distance

    print("Similarity:", similarity)

    if similarity >= 0.60:
        # Kirim perintah ke ESP8266
        requests.get("http://192.168.1.20/unlock")
        return jsonify({"match": True, "accuracy": float(similarity)})

    else:
        requests.get("http://192.168.1.20/incorrect")
        return jsonify({"match": False, "accuracy": float(similarity)})

app.run(host="0.0.0.0", port=5000)
