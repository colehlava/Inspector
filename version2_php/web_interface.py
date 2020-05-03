# web_interface.py
# Reads frame from video camera, converts it to jpg, writes it to file.

import cv2, os, time

# main
def main():

    videoFeed = cv2.VideoCapture(0)

    while True:
        # Capture frame
        ret, frame = videoFeed.read()

        # Encode the frame into jpg format
        (flag, jpg_image) = cv2.imencode(".jpg", frame)

        # Write jpg image to file
        with open('current_frame.jpg', 'wb') as jf:
            jf.write(jpg_image)

        os.system('sudo cp current_frame.jpg index.html /var/www/html/')
        time.sleep(1)




# Begin program
if __name__ == '__main__':
    main()

