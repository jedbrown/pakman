#ifndef ABSTRACTWORKERHANDLER_H
#define ABSTRACTWORKERHANDLER_H

#include <string>

#include "../types.h"

class AbstractWorkerHandler
{

    public:

        // Construct from command string and input string
        AbstractWorkerHandler(const cmd_t& command, const std::string& input_string);

        // Virtual destructor
        virtual ~AbstractWorkerHandler() = default;

        // Virtual function terminate() to prematurely terminate Worker
        virtual void terminate() = 0;

        // Virtual function isDone() to check whether Worker has finished
        virtual bool isDone() = 0;

        // Function getOutput() to get output of running command
        // Running this command before Worker is finished will result in
        // an error, so always check with isDone() first.
        std::string getOutput();

    protected:

        // Command string
        const cmd_t m_command;

        // Input string
        const std::string m_input_string;

        // Output buffer
        std::string m_output_buffer;
};

#endif // ABSTRACTWORKERHANDLER_H