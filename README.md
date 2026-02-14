# Motion Control Collection

A collection of motion-controlled games where players control the action using body movements detected through a webcam. Uses OpenPose for pose detection and SDL2 for graphics.

## Games Included

| # | Game | Description |
|---|------|-------------|
| 1 | **Pickleball** | Classic paddle game with AI opponent |
| 2 | **Boxing** | Punch targets that appear on screen |
| 3 | **Tennis** | Swing your arms to return the ball |
| 4 | **Archery** | Draw back and aim to hit targets |
| 5 | **Rhythm Dance** | Match on-screen poses to the beat (Just Dance style) |

## Purpose

This project serves as a proof-of-concept for **gesture-based gaming and motion control interfaces**. It demonstrates how computer vision and machine learning can be used to create intuitive, accessible, and engaging user experiences without requiring specialized hardware like motion controllers or VR headsets.

### Key Goals:
- **Accessibility**: Enable gaming through natural body movements
- **Low-Cost Motion Control**: Use only a standard webcam instead of expensive motion capture equipment
- **Real-Time Processing**: Show that pose estimation can run fast enough for interactive gaming
- **Educational**: Provide a working example of integrating OpenPose with game development

## How to Use

### Prerequisites
- A computer with a webcam (for motion controls)
- Linux, macOS, or Windows with WSL

### Installation

1. Navigate to the project directory:
   ```bash
   cd "motion control collection"
   ```

2. Run the setup script (this will install dependencies and download required AI models):
   ```bash
   ./build.sh
   ```

3. Start the game:
   ```bash
   ./output/motion_control
   ```

### Tips for Best Results
- Position yourself so your upper body is visible in the camera
- Ensure good lighting in your room
- Stand about 4-6 feet from the camera
- Wear contrasting colors from your background

## Menu Controls

| Input | Action |
|-------|--------|
| Arrow Keys / W/S | Select game |
| ENTER / SPACE | Start selected game |
| 1-5 | Quick select game |
| Q | Quit |

## Universal In-Game Controls

These controls work in all games:

| Input | Action |
|-------|--------|
| ESC | Return to menu |
| Q | Quit game |

---

## Game 1: Pickleball

### Overview
A classic paddle-and-ball game where you face off against an AI opponent. Control your paddle using your arm movements to hit the ball back and forth across a pickleball court. The AI adapts to your play style and predicts ball trajectories.

### How It Works
- Your paddle is on the left side of the screen
- The AI opponent controls the paddle on the right
- The ball bounces between the two paddles
- Move your arms up and down to control your paddle's vertical position
- The game tracks your wrist position to determine paddle height

### Gameplay
1. Press SPACE to serve the ball
2. Move your arms up/down to position your paddle
3. Hit the ball past your opponent to score
4. First to reach the target score wins
5. The court features "kitchen" zones near the net (just like real pickleball!)

### Scoring
- You score when the AI misses the ball
- The AI scores when you miss the ball
- Current score is displayed at the top of the screen

### Controls

| Input | Action |
|-------|--------|
| **Body Movement** | Move arms up/down to control paddle height |
| SPACE | Serve the ball / Start game |
| W / Up Arrow | Move paddle up (keyboard fallback) |
| S / Down Arrow | Move paddle down (keyboard fallback) |
| R | Reset score to 0-0 |

### Strategy Tips
- Watch the ball's trajectory and position yourself early
- The AI gets faster as the game progresses
- Use angles by hitting the ball with the edge of your paddle

---

## Game 2: Boxing

### Overview
A target-punching game where you throw punches at targets that appear on screen. Both hands are tracked independently, so you can punch with your left and right hands just like real boxing!

### How It Works
- Targets appear at random positions on the screen
- Your left and right hands are tracked separately
- Move your fists to the target locations to "punch" them
- Targets disappear when successfully punched
- New targets spawn continuously

### Gameplay
1. Stand in a boxing stance with both fists raised
2. Targets will appear on screen as colored circles
3. Move your left or right hand to overlap with a target
4. The target disappears when you successfully punch it
5. Try to punch as many targets as possible!

### Scoring
- Each successful punch earns points
- Faster punches may earn bonus points
- Your score and punch count are displayed on screen

### Controls

| Input | Action |
|-------|--------|
| **Left Hand Movement** | Control left fist position |
| **Right Hand Movement** | Control right fist position |
| W/A/S/D | Move left hand (keyboard fallback) |
| Mouse Movement | Move right hand (keyboard fallback) |
| Mouse Click | Punch with right hand (keyboard fallback) |

### Workout Tips
- Keep your guard up between punches
- Use quick jabs for nearby targets
- Cross your body for targets on the opposite side
- Great cardio workout!

---

## Game 3: Tennis

### Overview
A tennis game where you control your racket position using arm movements. Return the ball over the net to keep the rally going against an AI opponent.

### How It Works
- Your racket is positioned based on your arm height
- The ball travels across the court between you and the AI
- Position your racket to intercept the incoming ball
- The ball will return in the direction based on where it hits your racket

### Gameplay
1. The ball is served automatically
2. Move your arms up/down to position your racket
3. When the ball reaches your side, your racket must be in position to return it
4. Hit the ball past your opponent to score
5. Rally continues until someone misses

### Scoring
- Points are awarded when your opponent fails to return the ball
- Traditional tennis scoring is displayed
- Match continues until a winner is determined

### Controls

| Input | Action |
|-------|--------|
| **Body Movement** | Move arms to control racket height |
| W / Up Arrow | Move racket up (keyboard fallback) |
| S / Down Arrow | Move racket down (keyboard fallback) |

### Playing Tips
- Anticipate where the ball will be, not where it is
- The racket position affects the return angle
- Watch the opponent's position to find gaps

---

## Game 4: Archery

### Overview
A target shooting game where you draw back a virtual bow and aim at targets. Uses a realistic draw-and-release mechanic where you pull back to charge your shot and release to fire.

### How It Works
- Your arm position controls the aim direction (up/down)
- A "draw" motion (pulling hands apart) charges your shot
- Releasing the draw fires the arrow
- Arrows travel in an arc affected by gravity
- Hit the bullseye for maximum points!

### Gameplay
1. Targets appear at various distances
2. Move your arms up/down to aim
3. Perform a drawing motion (like pulling back a real bow string)
4. The longer you hold, the more power builds up
5. Release to fire the arrow
6. Try to hit the center of the target for bonus points!

### Scoring
- Bullseye (center): Maximum points
- Inner rings: High points
- Outer rings: Lower points
- Miss: No points
- Consecutive bullseyes may award combo bonuses

### Controls

| Input | Action |
|-------|--------|
| **Arm Position** | Aim up/down by raising/lowering arms |
| **Draw Motion** | Pull hands apart to draw the bow |
| **Release** | Bring hands together to fire |
| W / Up Arrow | Aim up (keyboard fallback) |
| S / Down Arrow | Aim down (keyboard fallback) |
| SPACE (hold) | Draw the bow (keyboard fallback) |
| SPACE (release) | Fire the arrow (keyboard fallback) |

### Accuracy Tips
- Take your time to aim - rushing leads to misses
- Account for arrow drop at longer distances
- Keep your arms steady while aiming
- Practice the draw-and-release timing

---

## Game 5: Rhythm Dance

### Overview
A Just Dance-style rhythm game where you match on-screen poses in time with the music. Strike the target poses when the beat hits to score points. The better your timing and pose accuracy, the higher your score!

### How It Works
- Target poses appear on screen showing a stick figure position
- A timing indicator shows when you need to match the pose
- Your body position is tracked in real-time
- The game compares your pose to the target pose
- Score is based on how closely you match and your timing

### Gameplay
1. Music plays with a rhythmic beat
2. Target poses scroll or appear on screen
3. When the timing indicator reaches the target, strike the pose
4. Hold the pose briefly for the game to register it
5. Move to the next pose and repeat
6. Chain together successful poses for combo bonuses!

### Pose Types
- **Arms Up**: Raise both arms above your head
- **Arms Out**: Extend arms to the sides (T-pose)
- **Arms Down**: Lower both arms to your sides
- **Left Arm Up**: Raise only your left arm
- **Right Arm Up**: Raise only your right arm
- **Mixed Poses**: Combinations of arm positions

### Scoring
- **Perfect**: Exact pose match with perfect timing
- **Great**: Good pose match with good timing
- **Good**: Acceptable pose match
- **Miss**: Pose not recognized or bad timing
- Combo multiplier increases with consecutive successful poses

### Controls

| Input | Action |
|-------|--------|
| **Full Body Movement** | Match the target poses shown on screen |
| **Both Arms Tracked** | Left and right arm positions are compared to target |

### Dance Tips
- Watch the upcoming poses to prepare
- Move smoothly between poses
- Exaggerate your movements for better detection
- Stay centered in the camera frame
- Have fun and don't be afraid to dance!

---

## Project Structure

```
motion control collection/
├── src/                         # Source files
│   ├── main.cpp                 # Entry point
│   ├── common.h/cpp             # Shared utilities (PoseDetector, drawing, colors)
│   ├── menu.h/cpp               # Main menu
│   ├── pickleball_game.h/cpp    # Pickleball game
│   ├── boxing_game.h/cpp        # Boxing game
│   ├── tennis_game.h/cpp        # Tennis game
│   ├── archery_game.h/cpp       # Archery game
│   └── rhythm_dance_game.h/cpp  # Rhythm Dance game
├── audio and sound effects/     # Music and sound effect files
│   ├── background.mp3           # Menu background music
│   ├── boxing_background.mp3    # Boxing background music
│   ├── rhythm_dance.mp3         # Rhythm Dance music
│   ├── archery_background.mp3   # Archery background music
│   └── arrow_shoot.mp3          # Arrow shooting sound effect
├── models/                      # OpenPose model files (downloaded by build.sh)
├── make/                        # CMake build directory
├── output/                      # Compiled executable
├── CMakeLists.txt               # Build configuration
└── build.sh                     # Setup and build script
```

## Technical Architecture

- **Language**: C++17
- **Graphics**: SDL2
- **Audio**: SDL2_mixer
- **Computer Vision**: OpenCV 4
- **Pose Detection**: OpenPose (via OpenCV DNN module)
- **Build System**: CMake

## How It Works

- **Body Tracking**: OpenPose model loaded from `models/` directory via OpenCV's DNN module
- **Motion Fallback**: If no OpenPose model files, uses motion detection based on frame differencing
- **Keyboard/Mouse Fallback**: If no camera available, all games support keyboard and mouse controls
- **Both Hands**: Boxing and Rhythm Dance track both hands for full control

## Real-World Applications

The technology demonstrated in this project has numerous practical applications beyond gaming:

### Physical Therapy and Rehabilitation
Motion-controlled games can make physical therapy exercises more engaging. Patients recovering from injuries could use similar systems to track range of motion, gamify exercises, and receive real-time feedback.

### Fitness and Exercise
Create interactive workout programs, track exercise form, and make home workouts more engaging through gamification.

### Accessibility Technology
Enable people with limited mobility to interact with computers using whatever movements they can perform, creating alternative input methods for those who cannot use traditional controllers.

### Education
Interactive learning experiences for children, teaching sports techniques with real-time feedback, and STEM education demonstrating AI and computer vision concepts.

### Sports Training
Analyze and improve athletic form, practice timing and coordination, and receive virtual coaching with real-time feedback.

## License

This project is for educational and demonstration purposes.

## Acknowledgments

- OpenPose by CMU Perceptual Computing Lab
- SDL2 by libsdl.org
- OpenCV by opencv.org
