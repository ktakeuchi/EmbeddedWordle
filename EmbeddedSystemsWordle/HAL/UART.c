/*
 * UART.c
 *
 *  Created on: Dec 31, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#include <HAL/Timer.h>
#include <HAL/UART.h>

/**
 * Initializes the UART module except for the baudrate generation
 * Except for baudrate generation, all other uart configuration should match that of basic_example_UART
 *
 * @param moduleInstance:   The eUSCI module used to construct this UART
 * @param port:             The port the UART is connected to
 * @param pins:             The Rx and Tx pins UART is connected to
 *
 * @return the completed UART object
 */
UART UART_construct(uint32_t moduleInstance, uint32_t port, uint32_t pins)
{
    // Create the UART struct
    UART uart;

    // Initialize auxiliary members of the struct
    // -------------------------------------------------------------------------
    uart.moduleInstance = moduleInstance;
    uart.port = port;
    uart.pins = pins;

    GPIO_setAsPeripheralModuleFunctionInputPin(
            uart.port, uart.pins, GPIO_PRIMARY_MODULE_FUNCTION);


    // TODO: Determine what other variables need to be initialized in the config struct
    // TODO: all uart config parameters except those related to baudrate generation
    uart.config.parity           = EUSCI_A_UART_NO_PARITY;    // No Parity
    uart.config.msborLsbFirst    = EUSCI_A_UART_LSB_FIRST;    // LSB First
    uart.config.uartMode         = EUSCI_A_UART_MODE;         // UART mode
    uart.config.dataLength       = EUSCI_A_UART_8_BIT_LEN;    //Data Length
    // Return the completed UART instance
    return uart;
}

/**
 * (Re)initializes and (re)enable the UART module to use a desired baudrate.
 *
 * @param uart_p        The pointer to the uart struct that needs a baudrate and should be enabled
 * @param baudChoice:   The new baud choice with which to update the module
 */
void UART_SetBaud_Enable(UART *uart_p, UART_Baudrate baudChoice)
{
    // We use the system clock for baudrate generation.
    // The processor clock runs at 48MHz in the project. In other words, system clock is 48MHz.
    uart_p->config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;


    // Fortunately for us, for all the baudrates we are working with with f = 48MHz, we use oversampling mode N>16
    uart_p->config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;


    // When using the baudChoice to index the array, we are effectively saying
    // that different baudrate choices correspond with different numbers
    // without requiring excessive amounts of if-statements.
    // TODO: fill these tables such that they work with the proper baudChoice. The first column is given for 9600 BPS
    uint32_t clockPrescalerMapping[NUM_BAUD_CHOICES] = {  312,  156, 78, 52};
    uint32_t firstModRegMapping[NUM_BAUD_CHOICES] =    {    8,     4,  2,  1};
    uint32_t secondModRegMapping[NUM_BAUD_CHOICES] =   { 0x00,  0x00,  0x00, 0x25};

    // Done TODO: Replace 0s with the correct statement. Use line 71 as your guide
    uart_p->config.clockPrescalar = clockPrescalerMapping[baudChoice];
    uart_p->config.firstModReg = firstModRegMapping[baudChoice];
    uart_p->config.secondModReg = secondModRegMapping[baudChoice];

    // TODO: initialize and enable uart instance (refer to the basic_example_UART project for guidance)
    UART_initModule(uart_p->moduleInstance, &uart_p->config);
    UART_enableModule(uart_p->moduleInstance);

}


// Not-a-real TODO: read this function and its comment to learn how to implement the other functions
// Notice how we use uart_p's module instance instead of the name of the instance from the basic example where we used no HAL
/**
 * Determines if the user has sent a UART data packet to the board by checking
 * the interrupt status of the proper system UART module.
 *
 * @param uart_p: The pointer to the UART instance with which to handle our operations.
 *
 * @return true if the user has entered a character, and false otherwise
 */
bool UART_hasChar(UART* uart_p)
{
    uint8_t interruptStatus = UART_getInterruptStatus(
            uart_p->moduleInstance, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);

    return (interruptStatus == EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
}

// TODO: Complete the UART_getChar() function.
// TODO: Write a descriptive comment explaining HOW the function is implemented.
//       In the implementation file, prefer explaining HOW the function is
//       implemented over simply WHAT the function does.
// This function takes in a UART pointer and receives the data, which is taken in as an unsigned int,
// which is then assigned to the uart's pointer's module instance. Returns a char
char UART_getChar(UART* uart_p)
{
    // TODO: change this
    return UART_receiveData(uart_p->moduleInstance);
}

// TODO: Complete the UART_canSend() function.
// TODO: Write a descriptive comment explaining HOW the function is implemented.
//       In the implementation file, prefer explaining HOW the function is
//       implemented over simply WHAT the function does.
// This function takes in a UART pointer, and sets an unsigned int to get the transfer status to see
// if it can transfer the data without any blockage. Returns a boolean
bool UART_canSend(UART* uart_p)
{
    uint8_t transferStatus = UART_getInterruptStatus(
    uart_p->moduleInstance, EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
    return (transferStatus == EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
}

// TODO: Complete the UART_sendChar() function.
// TODO: Write a descriptive comment explaining HOW the function is implemented.
//       In the implementation file, prefer explaining HOW the function is
//       implemented over simply WHAT the function does.
// This function takes in a UART pointer and a char, which then transmits the char c into the moduleInstance
// of the pointer. Returns nothing.
void UART_sendChar(UART* uart_p, char c)
{
  // TODO: add some lines here
    UART_transmitData(uart_p->moduleInstance, c);
    return;
}

