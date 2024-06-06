/*
 * Application.h
 *
 *  Created on: Dec 29, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#define MAX_LETTERS 5 // Max amount of letters per word
#define MAX_GUESSES 6 // Max amount of guesses for Player 2

#include <HAL/HAL.h>


enum _GameState
{
    TITLE_SCREEN, CREATE_WORD, GUESS_WORD
};
typedef enum _GameState GameState; // All application states

enum _LetterState
{
    FIRST = 0, SECOND = 1, THIRD = 2, FOURTH = 3, FIFTH = 4, END = 5
};
typedef enum _LetterState LetterState; // All Letter states

enum _GuessAmount
{
    ONE =0 , TWO = 1, THREE = 2, FOUR = 3, FIVE = 4, SIX = 5, RESULT = 6
};
typedef enum _GuessAmount GuessAmount; // Guess ammount states

struct _Application
{
    // Put your application members and FSM state variables here!
    // =========================================================================
    GameState state;
    UART_Baudrate baudChoice;
    LetterState letter;
    GuessAmount guess;

    bool firstCall;
    bool status;
    char word;
    unsigned char answer[MAX_LETTERS];
    unsigned char guessWord[MAX_LETTERS];
    int counter;
    int correct;
};
typedef struct _Application Application;

// Called only a single time - inside of main(), where the application is constructed
Application Application_construct();

// Called once per super-loop of the main application.
void Application_loop(Application* app, HAL* hal);

// Called whenever the UART module needs to be updated
void Application_updateCommunications(Application* app_p, HAL* hal);

// Interprets an incoming character and echoes back to terminal what kind of
// character was received (number, letter, or other)
char Application_interpretIncomingChar(char);

// Generic circular increment function
uint32_t CircularIncrement(uint32_t value, uint32_t maximum);

// Handle callback for each state of game
void Application_handleTitleScreen(Application* app_p, HAL* hal_p);
void Application_handleCreateWord(Application* app_p, HAL* hal_p);
void Application_handleGameScreen(Application* app_p, HAL* hal_p);
// Helper callback for each state (What's to appear)
void Application_showTitleScreen(Application* app_p, HAL* hal_p);
void Application_showCreateWord(Application* app_p, HAL* hal_p);
void Application_showGuessWord(Application* app_p, HAL* hal_p);
// All letter / guess related functions
char Application_upperCase(char rxChar);
void Application_letterUpdate(Application* app_p, HAL* hal_p);
void Application_letterDisplay(Application* app_p, HAL* hal_p);
void Application_guessDisplay(Application *app_p, HAL *hal_p);
void Application_wordleAlgo(Application *app_p, HAL *hal_p);
void Application_correctResult(Application *app_p, HAL *hal_p);
// UART related function
void Application_begin(Application* app_p, HAL* hal_p);
#endif /* APPLICATION_H_ */
