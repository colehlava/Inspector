#!/usr/bin/python3

# web_interface.py
# Creates html file to stream video feed to web browser.

import flask, cv2, threading, argparse, os, time

# Global variables
app = flask.Flask(__name__.split('.')[0])
videoFeed = cv2.VideoCapture(0)
outputFrame = None
frameLock = threading.Lock()

@app.route("/", methods=['GET', 'POST'])
def index():
    # Read button inputs from user
    if flask.request.method == 'POST':
        if flask.request.form.get('on') == 'on':
            with open('/home/pi/Documents/Inspector/Inspector/version1_flask/button_status.txt', 'w') as bs:
                bs.write("1")
        elif flask.request.form.get('off') == 'off':
            with open('/home/pi/Documents/Inspector/Inspector/version1_flask/button_status.txt', 'w') as bs:
                bs.write("0")

    # Return the rendered html file
    return flask.render_template("index.html")


@app.route("/key_input", methods=['GET', 'POST'])
def updateKeyInput():
    # Read key input from user and write most recent key to data file
    if flask.request.method == 'POST':
        data = flask.request.get_json()
        inputKey = data['keyVal']

        # Map input key to adws control scheme and write to file
        if inputKey == 'Left':
            with open('/home/pi/Documents/Inspector/Inspector/version1_flask/key_input.txt', 'w') as ki:
                ki.write('a')
        elif inputKey == 'Right':
            with open('/home/pi/Documents/Inspector/Inspector/version1_flask/key_input.txt', 'w') as ki:
                ki.write('d')
        elif inputKey == 'Up':
            with open('/home/pi/Documents/Inspector/Inspector/version1_flask/key_input.txt', 'w') as ki:
                ki.write('w')
        elif inputKey == 'Down':
            with open('/home/pi/Documents/Inspector/Inspector/version1_flask/key_input.txt', 'w') as ki:
                ki.write('s')


    # Return the rendered html file
    return flask.render_template("index.html")


@app.route("/speed_slider", methods=['GET', 'POST'])
def updateSpeed():
    # Read speed slider input from user
    if flask.request.method == 'POST':
        with open('/home/pi/Documents/Inspector/Inspector/version1_flask/speed.txt', 'w') as s:
            s.write(flask.request.form['speed'])

    # Return the rendered html file
    return flask.render_template("index.html")


# Stream video from web cam
def streamVideo():
    global videoFeed, outputFrame, frameLock

    while True:
        # Capture frame
        ret, frame = videoFeed.read()

        # Acquire the lock, update the output frame, then release the lock
        with frameLock:
            outputFrame = frame.copy()


# Generate a jpg file of the image and format into a byte array
def generate():
    # Grab global references to the output frame and frame lock variables
    global outputFrame, frameLock

    # Loop over frames from the output stream
    while True:
        # Wait until the lock is acquired
        with frameLock:
            if outputFrame is None:
                continue

            # Encode the frame into jpg format
            (flag, encodedImage) = cv2.imencode(".jpg", outputFrame)

        # @NOTE make sure this is correct rv logic
        # Ensure the frame was successfully encoded
        if not flag:
            continue

        # Yield a byte array of the .jpg frame
        yield(b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')


# Generate object for web application
@app.route("/video_feed")
def video_feed():
    # Return the generated response and media type
    return flask.Response(generate(), mimetype = "multipart/x-mixed-replace; boundary=frame")


# main
def main():
    # Parse command line arguments
    ap = argparse.ArgumentParser()
    ap.add_argument("-i", "--ip", type=str, default="192.168.1.101", help="ip address of the device")
    ap.add_argument("-o", "--port", type=int, default="5005", help="ephemeral port number of the server (1024 to 65535)")
    args = vars(ap.parse_args())

    # Write initial values to data files
    with open('/home/pi/Documents/Inspector/Inspector/version1_flask/button_status.txt', 'w') as bf:
        bf.write("0")
    with open('/home/pi/Documents/Inspector/Inspector/version1_flask/speed.txt', 'w') as sf:
        sf.write("50")
    with open('/home/pi/Documents/Inspector/Inspector/version1_flask/key_input.txt', 'w') as kf:
        kf.write("w")

    # Start thread to stream video
    video_thread = threading.Thread(target=streamVideo, args=())
    video_thread.start()

    # Start the flask web application
    app.run(host=args["ip"], port=args["port"], debug=True,
        threaded=True, use_reloader=False)


# Begin program
if __name__ == '__main__':
    time.sleep(8)
    os.system('sudo /home/pi/Documents/Inspector/Inspector/version1_flask/a.out &')
    main()

