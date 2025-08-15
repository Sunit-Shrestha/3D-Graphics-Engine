import mediapipe as mp
import cv2
import time
import math
from pynput.mouse import Controller
from pynput.keyboard import Controller as KeyboardController, Key

# Initialize MediaPipe Hand Tracking
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(min_detection_confidence=0.7, min_tracking_confidence=0.7)
mp_drawing = mp.solutions.drawing_utils

mouse = Controller()  # For controlling the mouse
keyboard = KeyboardController()  # For controlling the keyboard

# Track previous positions for both hands
prev_right_index = None
prev_left_index = None

# Timing for key presses to avoid spam
last_key_time = 0
key_cooldown = 0.01  # 75ms between key presses (4x faster)

# Start webcam feed
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

print("Press 'Esc' to exit.")

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb_frame)

    if results.multi_hand_landmarks and results.multi_handedness:
        left_hand_pinch = False
        right_hand_pinch = False
        right_index_pos = None
        left_index_pos = None

        for hand_landmarks, handedness in zip(results.multi_hand_landmarks, results.multi_handedness):
            mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)

            hand_label = handedness.classification[0].label  # 'Left' or 'Right'
            thumb_tip = hand_landmarks.landmark[4]
            index_tip = hand_landmarks.landmark[8]

            # Detect pinch (distance between thumb and index)
            distance = math.sqrt((thumb_tip.x - index_tip.x)**2 + (thumb_tip.y - index_tip.y)**2)

            if hand_label == "Left":
                left_hand_pinch = distance < 0.05  # tweak threshold for pinch
                left_index_pos = (index_tip.x, index_tip.y)
            elif hand_label == "Right":
                right_hand_pinch = distance < 0.05  # tweak threshold for pinch
                right_index_pos = (index_tip.x, index_tip.y)

        current_time = time.time()
        
        # Left hand pinch: send arrow keys based on right index finger displacement
        if left_hand_pinch and right_index_pos and current_time - last_key_time > key_cooldown:
            if prev_right_index is not None:
                dx = right_index_pos[0] - prev_right_index[0]
                dy = right_index_pos[1] - prev_right_index[1]

                # Determine direction based on displacement (with threshold to avoid noise)
                threshold = 0.02
                if abs(dx) > threshold or abs(dy) > threshold:
                    if abs(dx) > abs(dy):  # Horizontal movement
                        if dx > 0:
                            keyboard.press(Key.right)
                            keyboard.release(Key.right)
                            print("Arrow Right")
                        else:
                            keyboard.press(Key.left)
                            keyboard.release(Key.left)
                            print("Arrow Left")
                    else:  # Vertical movement
                        if dy > 0:
                            keyboard.press(Key.down)
                            keyboard.release(Key.down)
                            print("Arrow Down")
                        else:
                            keyboard.press(Key.up)
                            keyboard.release(Key.up)
                            print("Arrow Up")
                    
                    last_key_time = current_time

            prev_right_index = right_index_pos
        elif not left_hand_pinch:
            prev_right_index = None

        # Right hand pinch: send h/j/k/l keys based on left index finger displacement
        if right_hand_pinch and left_index_pos and current_time - last_key_time > key_cooldown:
            if prev_left_index is not None:
                dx = left_index_pos[0] - prev_left_index[0]
                dy = left_index_pos[1] - prev_left_index[1]

                # Determine direction based on displacement (with threshold to avoid noise)
                threshold = 0.02
                if abs(dx) > threshold or abs(dy) > threshold:
                    if abs(dx) > abs(dy):  # Horizontal movement
                        if dx > 0:
                            keyboard.press('l')  # right
                            keyboard.release('l')
                            print("Key: l (right)")
                        else:
                            keyboard.press('h')  # left
                            keyboard.release('h')
                            print("Key: h (left)")
                    else:  # Vertical movement
                        if dy > 0:
                            keyboard.press('j')  # down
                            keyboard.release('j')
                            print("Key: j (down)")
                        else:
                            keyboard.press('k')  # up
                            keyboard.release('k')
                            print("Key: k (up)")
                    
                    last_key_time = current_time

            prev_left_index = left_index_pos
        elif not right_hand_pinch:
            prev_left_index = None

    cv2.imshow("Hand Gesture Control", frame)
    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
