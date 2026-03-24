# from flask import Flask, Response
# import threading
# import time
# import mss
# from PIL import Image
# import io


# app = Flask(__name__)

# frame_lock = threading.Lock()
# frame_data = None


# TARGET_W = 320
# TARGET_H = 240
# JPEG_QUALITY = 40   # lower quality = faster transfer. 40 should give 200-300 ms per frame in mirroring
# CAPTURE_FPS = 20    # 50 milliseconds per frame

# interval = 1.0 / CAPTURE_FPS

# def capture_screen():
#     global frame_data
    
#     with mss.mss() as sct:
#         monitor = sct.monitors[1] 
#         while 1:
#             start = time.time()
#             try:

#                 screenshot = sct.grab(monitor)

#                 image = Image.frombytes("RGB",screenshot.size,screenshot.rgb)
#                 image = image.resize((TARGET_W,TARGET_H),Image.LANCZOS)

#                 buffer = io.BytesIO()

#                 image.save(buffer,"JPEG",quality=JPEG_QUALITY,optimize=True)

#                 buffer.seek(0)
#                 bytes = buffer.read()

#                 with frame_lock:
#                     frame_data = bytes

#                 end = time.time() - start
#                 if(end < interval): time.sleep(interval-end)

#             except Exception as exc:
#                 print(f"[capture] an error occured: {exc}")

#                 end = time.time() - start
#                 if(end < interval): time.sleep(interval-end)


# @app.route("/frame.jpg")
# def get_frame():
#     with frame_lock:
#         data = frame_data
#     if data is None:
#         return "No frame yet", 503
#     return Response(data, mimetype="image/jpeg",
#                     headers={"Cache-Control": "no-store"})



# if __name__ == "__main__":
#     print(f"[server] capturing at {CAPTURE_FPS} fps → {TARGET_W}x{TARGET_H} JPEG q{JPEG_QUALITY}")
#     print("[server] ESP32 should GET  http://<YOUR_PC_IP>:8000/frame.jpg")
#     t = threading.Thread(target=capture_screen, daemon=True)
#     t.start()
#     app.run(host="0.0.0.0", port=8000, threaded=True)