import mediapipe as mp
import cv2
import time
import math
import socket  # Import socket for UDP communication

# Initialize MediaPipe Hand Tracking
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(min_detection_confidence=0.7, min_tracking_confidence=0.7)
mp_drawing = mp.solutions.drawing_utils

# Socket setup
UDP_IP = "127.0.0.1"  # Localhost (same device)
UDP_PORT = 5005       # Port to send data
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Gesture recognition logic
def recognize_fast_swipe(prev_x, curr_x, prev_time, curr_time):
    if prev_x is not None and curr_x is not None and prev_time is not None and curr_time is not None:
        time_diff = curr_time - prev_time
        if time_diff > 0:  # Ensure no division by zero
            # Calculate speed (change in x over time)
            speed = abs(curr_x - prev_x) / time_diff
            if curr_x < prev_x and speed > 1.5:  # Threshold for fast left swipe
                return "left_swipe"
            elif curr_x > prev_x and speed > 1.5:  # Threshold for fast right swipe
                return "right_swipe"
    return None

def recognize_fast_pinch(prev_distance, curr_distance, prev_time, curr_time):
    if prev_distance is not None and curr_distance is not None and prev_time is not None and curr_time is not None:
        time_diff = curr_time - prev_time
        if time_diff > 0:  # Ensure no division by zero
            # Calculate speed (change in distance over time)
            speed = abs(curr_distance - prev_distance) / time_diff
            if curr_distance < prev_distance and speed > 1.0:  # Increased threshold for fast pinch in
                return "pinch_in"
            elif curr_distance > prev_distance and speed > 1.0:  # Increased threshold for fast pinch out
                return "pinch_out"
    return None

# Start webcam feed
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

prev_x = None  # Track the previous x-coordinate of the wrist
prev_distance = None  # Track the previous distance between thumb and index finger
prev_time = None  # Track the previous timestamp
gesture_state = "idle"  # Track the current gesture state
last_gesture_time = 0  # Track the time of the last registered gesture
cooldown_time = 1  # Cooldown time in seconds
print("Press 'Esc' to exit.")
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read frame.")
        break

    # Flip the frame horizontally for a mirrored view
    frame = cv2.flip(frame, 1)
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb_frame)

    curr_time = time.time()  # Get the current timestamp

    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
            
            # Get the current x-coordinate of the wrist (landmark 0)
            curr_x = hand_landmarks.landmark[0].x
            
            # Detect fast swipe gesture
            gesture = recognize_fast_swipe(prev_x, curr_x, prev_time, curr_time)
            if gesture in ["left_swipe", "right_swipe"] and curr_time - last_gesture_time > cooldown_time:
                print(f"Gesture detected: {gesture}")
                sock.sendto(gesture.encode(), (UDP_IP, UDP_PORT))  # Send gesture data
                last_gesture_time = curr_time  # Update the last gesture time
                gesture_state = gesture
            
            # Calculate the distance between thumb tip (landmark 4) and index finger tip (landmark 😎
            thumb_tip = hand_landmarks.landmark[4]
            index_tip = hand_landmarks.landmark[8]
            curr_distance = math.sqrt((thumb_tip.x - index_tip.x) * 2 + (thumb_tip.y - index_tip.y) * 2)
            
            # Detect fast pinch gesture
            gesture = recognize_fast_pinch(prev_distance, curr_distance, prev_time, curr_time)
            if gesture in ["pinch_in", "pinch_out"] and curr_time - last_gesture_time > cooldown_time:
                print(f"Gesture detected: {gesture}")
                sock.sendto(gesture.encode(), (UDP_IP, UDP_PORT))  # Send gesture data
                last_gesture_time = curr_time  # Update the last gesture time
                gesture_state = gesture
            
            # Update the previous x-coordinate, distance, and timestamp
            prev_x = curr_x
            prev_distance = curr_distance
            prev_time = curr_time

        # Reset the gesture state when the hand stops moving
        if gesture_state in ["left_swipe", "right_swipe", "pinch_in", "pinch_out"]:
            gesture_state = "idle"

    cv2.imshow("Hand Gesture Recognition", frame)
    if cv2.waitKey(1) & 0xFF == 27:  # Press 'Esc' to exit
        break

cap.release()
cv2.destroyAllWindows()