/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* HAL and Application includes */
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void InitNonBlockingLED()
{
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void PollNonBlockingLED()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

/**
 * The main entry point of your project. The main function should immediately
 * stop the Watchdog timer, call the Application constructor, and then
 * repeatedly call the main super-loop function. The Application constructor
 * should be responsible for initializing all hardware components as well as all
 * other finite state machines you choose to use in this project.
 *
 * THIS FUNCTION IS ALREADY COMPLETE. Unless you want to wordorarily experiment
 * with some behavior of a code snippet you may have, we DO NOT RECOMMEND
 * modifying this function in any way.
 */
int main(void)
{
    // Stop Watchdog Timer - THIS SHOULD ALWAYS BE THE FIRST LINE OF YOUR MAIN
    WDT_A_holdTimer();

    // Initialize the system clock and background hardware timer, used to enable
    // software timers to time their measurements properly.
    InitSystemTiming();

    // Initialize the main Application object and HAL object
    HAL hal = HAL_construct();
    Application app = Application_construct();

    // Do not remove this line. This is your non-blocking check.
    InitNonBlockingLED();

    // Main super-loop! In a polling architecture, this function should call
    // your main FSM function over and over.
    while (true)
    {
        // Do not remove this line. This is your non-blocking check.
        PollNonBlockingLED();
        HAL_refresh(&hal);
        Application_loop(&app, &hal);
    }
}

/**
 * A helper function which increments a value with a maximum. If incrementing
 * the number causes the value to hit its maximum, the number wraps around
 * to 0.
 */
uint32_t CircularIncrement(uint32_t value, uint32_t maximum)
{
    return (value + 1) % maximum;
}

/**
 * The main constructor for your application. This function should initialize
 * each of the FSMs which implement the application logic of your project.
 *
 * @return a completely initialized Application object
 */
Application Application_construct()
{
    Application app;

    // Initialize local application state variables here!
    app.baudChoice = BAUD_9600;
    app.firstCall = true;
    app.state = TITLE_SCREEN;
    app.letter = FIRST;
    app.guess = ONE;
    app.counter = 0;
    app.correct = 0;
    app.word = 0;

    return app;
}

/**
 * The main super-loop function of the application. We place this inside of a
 * single infinite loop in main. In this way, we can model a polling system of
 * FSMs. Every cycle of this loop function, we poll each of the FSMs one time,
 * followed by refreshing all inputs to the system through a convenient
 * [HAL_refresh()] call.
 *
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */
void Application_loop(Application *app_p, HAL *hal_p)
{
    if (app_p->firstCall)
    {
        Application_showTitleScreen(app_p, hal_p); // Display Title screen during its first time launch
        Application_updateCommunications(app_p, hal_p); // When app is just called, update communication
    }
    if (Button_isTapped(&hal_p->boosterpackS2))
    {
        Application_updateCommunications(app_p, hal_p); // Update Baudrate

    }
    if (Button_isTapped(&hal_p->boosterpackS1))
    {
        if ((app_p->state == CREATE_WORD) && (app_p->letter == END))
        {
            // Transition from Create Word state to Guess Word State
            app_p->state = GUESS_WORD;
            app_p->letter = FIRST;
            app_p->counter = 0;
            Application_showGuessWord(app_p, hal_p); // Display Guess Word state
        }
        if ((app_p->state == GUESS_WORD) && (app_p->letter == END))
        {
            // Run through Wordle algorithm and display the boxes
            Application_wordleAlgo(app_p, hal_p);
            // Reset the letter state and update guess state to next guess
            app_p->letter = FIRST;
            app_p->counter = 0;
            app_p->guess = (GuessAmount) ((int) app_p->guess + 1);
        }
    }
    // When UART has a character in the terminal, is goes to changing the states of the app
    if (UART_hasChar(&hal_p->uart))
    {
        Application_begin(app_p, hal_p);
        switch (app_p->state)
        {
        case TITLE_SCREEN:
            Application_handleTitleScreen(app_p, hal_p); // Go to the title screen function
            break;

        case CREATE_WORD:
            Application_handleCreateWord(app_p, hal_p); // Go to create word function
            break;

        case GUESS_WORD:
            Application_handleGameScreen(app_p, hal_p); // Go to guess word function
            break;

        default:
            break;
        }
    }
}

/**
 * Updates which LEDs are lit and what baud rate the UART module communicates
 * with, based on what the application's baud choice is at the time this
 * function is called.
 *
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */
void Application_updateCommunications(Application *app_p, HAL *hal_p)
{
    // When this application first loops, the proper LEDs aren't lit. The
    // firstCall flag is used to ensure that the
    if (app_p->firstCall)
    { // The beginning; Once it's called, you set to false
        app_p->firstCall = false;
    }

    // When BoosterPack S2 is tapped, circularly increment which baud rate is used.
    else
    {
        uint32_t newBaudNumber = CircularIncrement((uint32_t) app_p->baudChoice,
                                                   NUM_BAUD_CHOICES);
        app_p->baudChoice = (UART_Baudrate) newBaudNumber;
    }

    // Start/update the baud rate according to the one set above.
    UART_SetBaud_Enable(&hal_p->uart, app_p->baudChoice);

    // Based on the new application choice, turn on the correct LED.
    // To make your life easier, we recommend turning off all LEDs before
    // selectively turning back on only the LEDs that need to be relit.
    // -------------------------------------------------------------------------
    LED_turnOff(&hal_p->launchpadLED2Red);
    LED_turnOff(&hal_p->launchpadLED2Green);
    LED_turnOff(&hal_p->launchpadLED2Blue);

    // TODO: Turn on all appropriate LEDs according to the tasks below.
    switch (app_p->baudChoice)
    {
    // When the baud rate is 9600, turn on Launchpad LED Red
    case BAUD_9600:
        LED_turnOn(&hal_p->launchpadLED2Red);
        break;

        // TODO: When the baud rate is 19200, turn on Launchpad LED Green
    case BAUD_19200:
        LED_turnOn(&hal_p->launchpadLED2Green);
        break;

        // TODO: When the baud rate is 38400, turn on Launchpad LED Blue
    case BAUD_38400:
        LED_turnOn(&hal_p->launchpadLED2Blue);
        break;

        // TODO: When the baud rate is 57600, turn on all Launchpad LEDs (illuminates white)
    case BAUD_57600:
        LED_turnOn(&hal_p->launchpadLED2Red);
        LED_turnOn(&hal_p->launchpadLED2Green);
        LED_turnOn(&hal_p->launchpadLED2Blue);
        break;

        // In the default case, this program will do nothing.
    default:
        break;
    }
}

/**
 * Interprets a character which was incoming and returns an interpretation of
 * that character. If the input character is a letter, it return L for Letter, if a number
 * return N for Number, and if something else, it return O for Other.
 *
 * @param rxChar: Input character
 * @return :  Output character
 */
char Application_interpretIncomingChar(char rxChar)
{
    // The character to return back to sender. By default, we assume the letter
    // to send back is an 'O' (assume the character is an "other" character)
    char txChar = 'O';

    // Numbers - if the character entered was a number, transfer back an 'N'
    if (rxChar >= '0' && rxChar <= '9')
    {
        txChar = 'N';
    }

    // Letters - if the character entered was a letter, transfer back an 'L'
    if ((rxChar >= 'a' && rxChar <= 'z') || (rxChar >= 'A' && rxChar <= 'Z'))
    {
        txChar = 'L';
    }

    // Backspace
    if ((rxChar == 0x8))
    {
        txChar = 'B';
    }
    return txChar;
}

/**
 * Handle Title Screen state
 */

void Application_handleTitleScreen(Application *app_p, HAL *hal_p)
{
    if (app_p->word != 0) // Runs through when there is app.word has an input
    {
        // Change states and display create word state
        app_p->state = CREATE_WORD;
        Application_showCreateWord(app_p, hal_p);
    }
}

/**
 * Create Word function: Creates word based on its letter state
 */
void Application_handleCreateWord(Application *app_p, HAL *hal_p)
{
    if (UART_canSend(&hal_p->uart))
    {
        // Each state represents the position of the imaginary cursor
        // Once it reaches end, you can still backspace, or if the button 1 is pressed it will switch states
        switch (app_p->letter)
        {
        case FIRST:
            Application_letterUpdate(app_p, hal_p);
            break;

        case SECOND:
            Application_letterUpdate(app_p, hal_p);
            break;

        case THIRD:
            Application_letterUpdate(app_p, hal_p);
            break;

        case FOURTH:
            Application_letterUpdate(app_p, hal_p);
            break;

        case FIFTH:
            Application_letterUpdate(app_p, hal_p);
            break;

        case END:
            Application_letterUpdate(app_p, hal_p);
            break;
        default:
            break;
        }
    }
}
/**
 * Create guessing state function: goes through each possible case of guesses
 */
void Application_handleGameScreen(Application *app_p, HAL *hal_p)
{
    if (UART_canSend(&hal_p->uart))
    {
        // Each state represents the number of guesses player 2 is on
        // Dependent: If each case reaches a certain count, state will change to result
        // Auto: Once it reaches case six and Button 1 is pressed, it will
        // return the resulting screen (main loop)
        switch (app_p->guess)
        {
        case ONE:
            Application_handleCreateWord(app_p, hal_p);
            break;

        case TWO:
            Application_handleCreateWord(app_p, hal_p);
            break;

        case THREE:
            Application_handleCreateWord(app_p, hal_p);
            break;

        case FOUR:
            Application_handleCreateWord(app_p, hal_p);
            break;

        case FIVE:
            Application_handleCreateWord(app_p, hal_p);
            break;

        case SIX:
            Application_handleCreateWord(app_p, hal_p);
            break;

        default:
            break;
        }
    }
}
/**
 * Showing Title Screen state
 */
void Application_showTitleScreen(Application *app, HAL *hal_p)
{
    Graphics_clearDisplay(&hal_p->g_sContext);
    Graphics_setFont(&hal_p->g_sContext, &g_sFontCmss12b);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "WordMaster", -1, 25, 0,
    true);
    Graphics_setFont(&hal_p->g_sContext, &g_sFontFixed6x8);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Spring 22 Project 1", -1,
                        0, 15, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Kyle Takeuchi", -1, 0,
                        24, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "-----How To Play-----",
                        -1, 0, 35, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Player 1 Creates Word",
                        -1, 0, 47, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Player 2 has 6 tries ",
                        -1, 0, 59, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "to guess the word.", -1,
                        0, 71, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "BB1: Confirm Word", -1,
                        0, 83, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "BB2: Baudrate Select",
                        -1, 0, 95, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "UART: Type Word", -1, 0,
                        107, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "PRESS ANY KEY TO PLAY",
                        -1, 0, 119, true);
}

/**
 * Showing Create word state
 */
void Application_showCreateWord(Application *app, HAL *hal_p)
{
    Graphics_clearDisplay(&hal_p->g_sContext);
    Graphics_setFont(&hal_p->g_sContext, &g_sFontCmsc14);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "PLAYER 1", -1, 25, 0,
    true);
    Graphics_setFont(&hal_p->g_sContext, &g_sFontCmsc12);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Create Word", -1, 18, 18,
    true);
    Graphics_setFont(&hal_p->g_sContext, &g_sFontFixed6x8);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Word: ", -1, 30, 35,
    true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "------BAUDRATES------",
                        -1, 0, 47, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Red: 9600", -1, 0, 59,
    true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Green: 19200", -1, 0, 71,
    true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Blue: 38400", -1, 0, 83,
    true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "White: 57600", -1, 0, 95,
    true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "BB1: Confirm Word", -1,
                        0, 107, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "BB2: Change Baudrate",
                        -1, 0, 119, true);
}

/**
 * Showing Guess word state
 */
void Application_showGuessWord(Application *app, HAL *hal_p)
{
    Graphics_clearDisplay(&hal_p->g_sContext);
    Graphics_setFont(&hal_p->g_sContext, &g_sFontCmsc14);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Guess Word", -1, 20, 0,
    true);
    Graphics_setFont(&hal_p->g_sContext, &g_sFontFixed6x8);
}

char Application_upperCase(char rxChar)
{
    if (rxChar >= 'a' && rxChar <= 'z')
        return rxChar - 0x20;
    else
        return rxChar;
}

/**
 * Updates the letter in Application struct, dependent on if the state is create word or guess word
 * Also Returns and displays the letter in both the UART and LCD
 */
void Application_letterUpdate(Application *app_p, HAL *hal_p)
{
    char txChar = Application_interpretIncomingChar(app_p->word);
    UART_sendChar(&hal_p->uart, app_p->word);
    if ((txChar == 'L') && (app_p->letter != END))
    {
        if (app_p->state == CREATE_WORD) // In this state it will update the answer struct variable and this will remain unchanged throughout the whole run
        {
            app_p->answer[app_p->counter] = Application_upperCase(app_p->word); // Make everything upper, even if it is already upper
            Application_letterDisplay(app_p, hal_p);
            app_p->counter = app_p->counter + 1;
        }
        else if (app_p->state == GUESS_WORD) // Changes the guessWord char everytime / rewrites it
        {
            app_p->guessWord[app_p->counter] = Application_upperCase(
                    app_p->word); // Make everything upper, even if it is already upper
            Application_guessDisplay(app_p, hal_p);
            app_p->counter = app_p->counter + 1;
        }

        app_p->letter = (LetterState) ((int) app_p->letter + 1);
    }
    else if ((txChar == 'B'))
    {
        if (app_p->letter != FIRST)
        {
            app_p->letter = (LetterState) ((int) app_p->letter - 1);
            app_p->counter = app_p->counter - 1;
            if (app_p->state == CREATE_WORD)
            {
                app_p->answer[app_p->counter] = 0x20; // Replace with space (ASCII)
                Application_letterDisplay(app_p, hal_p);
            }
            else if (app_p->state == GUESS_WORD)
            {
                app_p->guessWord[app_p->counter] = 0x20;
                Application_guessDisplay(app_p, hal_p);
            }
        }
    }
}
/**
 * Displays letter given from UART and whatever is assigned from App struct
 */
void Application_letterDisplay(Application *app_p, HAL *hal_p)
{
    char word[1];
    int increment;
    word[0] = app_p->answer[app_p->counter];
    increment = app_p->counter;
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) word, 1,
                        (60 + (increment * 6)), 35, true);
}
/**
 * Displays guessing words from UART and whatever is assigned from App struct
 */
void Application_guessDisplay(Application *app_p, HAL *hal_p)
{
    char word[1];
    int increment;
    int vertical;
    word[0] = app_p->guessWord[app_p->counter];
    increment = app_p->counter;
    vertical = (int) app_p->guess;
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) word, 1,
                        13 + (increment * 6), 20 + (vertical * 12), true);
}

/**
 * Calls getChar function from UART class once during loop (As told from Piazza)
 */
void Application_begin(Application *app_p, HAL *hal_p)
{
    char rxChar = UART_getChar(&hal_p->uart);
    char txChar = Application_interpretIncomingChar(rxChar);
    app_p->word = rxChar;
}

// TWO FUNCTIONS FOR THE GUESSING PART: Algorithm and display: FIX THE SQUARES AND THE ALGORITHM FOR ADDING UP THE CORRECT AMOUNT
void Application_wordleAlgo(Application *app_p, HAL *hal_p)
{
    app_p->correct = 0; // Resets Correct app struct variable whenever WordleAlgo is called
    int i, j;
    int vert;
    vert = (int) app_p->guess; // Guess state
    Graphics_Rectangle R;
    // Reinitialize every time bc its not supposed to carry on guess by guess
    int correctAmt = 0; // Local variable to later set to correct app struct
    for (i = 0; i < MAX_LETTERS; i++)
    {
        bool status = false; // Used to determine if this position is already filled with a square or not, reinitializes every time it loops
        R.xMin = 55 + (i * 12);
        R.xMax = 62 + (i * 12);
        R.yMin = 18 + (vert * 12);
        R.yMax = 25 + (vert * 12);
        if (app_p->guessWord[i] == app_p->answer[i]) // Compares the guessingWord struct var to the answer letter by letter
        {
            // Green box
            Graphics_setForegroundColor(&hal_p->g_sContext,
            GRAPHICS_COLOR_GREEN);
            Graphics_fillRectangle(&hal_p->g_sContext, &R);
            status = true;
            correctAmt = correctAmt + 1; // When they get it right, it adds to the local variable
        }
        for (j = 0; j < MAX_LETTERS; j++) // Nested for loop to run through all answer letter by letter and compare to each letter from guessWord
        {
            if (status != true)
            {
                // Gray box
                Graphics_setForegroundColor(&hal_p->g_sContext,
                GRAPHICS_COLOR_GRAY);
                Graphics_fillRectangle(&hal_p->g_sContext, &R);
                if ((app_p->guessWord[i] == app_p->answer[j]))
                {
                    // Yellow Box
                    Graphics_setForegroundColor(&hal_p->g_sContext,
                    GRAPHICS_COLOR_YELLOW);
                    Graphics_fillRectangle(&hal_p->g_sContext, &R);
                    status = true; // Since it is filled with a color that's not gray
                }
            }
        }

    }
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE); // Reinitialize the foreground color for the text
    app_p->correct = app_p->correct + correctAmt; // Adds the correct Amt to a struct variable
    Application_correctResult(app_p, hal_p); // Dislays the conditions for winning, losing, or neither
}

/**
 * Displays correct Result at the bottom of the guess word state.
 */
void Application_correctResult(Application *app_p, HAL *hal_p)
{
    // This needs to be checked everycase.
    if (app_p->correct == 5) // If Player 2 guesses the correct answer within 6 tries
    {
        Graphics_setFont(&hal_p->g_sContext, &g_sFontCmsc14);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Player 2 Wins", -1,
                            13, 94, true);
        app_p->guess = RESULT;
    }
    else if ((app_p->guess == SIX) && (app_p->correct != 5)) // If Player 2 doesn't get it within 6 tries
    {
        Graphics_setFont(&hal_p->g_sContext, &g_sFontCmsc12);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Player 1 Wins", -1,
                            15, 94, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Word : ", -1, 15,
                            109, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) app_p->answer, 5, 65,
                            109, true);
        app_p->guess = RESULT;
    }
    // Everything else will just run through this without changing anything.

}
