# Wordle on Embedded Systems
## Project Summary
This project is an interactive word-guessing game designed to run on an embedded system. The game features a two-player mode where one player enters a word, and the other player has six attempts to guess it correctly.

## Key Features
- Initial Title Screen: Waits for any key press to transition to the word creation screen.
- Word Creation Screen: Accepts letter inputs (converted to uppercase) and ignores other characters except for backspace, which removes the last letter typed.
- Guessing Screen: Provides feedback for each guess with colored boxes indicating correct letters and positions (Green: correct position, Yellow: correct letter wrong position, Gray: letter not in word).
- Baud Rate Cycling: Changes baud rates between 9600, 19200, 38400, and 57600 using Button 2, indicated by corresponding LED colors (Red, Green, Blue, White).
- Win Conditions: Displays a message for the winning player depending on the guess outcome.

## System Architecture
### Finite State Machine:
- Three states: Title Screen, Create Word, Guess Word.
- Each state handles specific functions and transitions based on user inputs.
### Embedded System Components: Utilizing MSP432
- CPU: Central processing unit managing all components.
- UART: Handles communication with the terminal.
- LCD: Displays game screens and feedback.
- Buttons (BB1, BB2): Used for transitioning states and changing baud rates.
- LEDs (LL1, LL2): Indicate current baud rate and game states.
