#include <string>
#include <thread>

#include "types.h"

#include "mpi_utils.h"
#include "mpi_common.h"
#include "spawn.h"
#include "PersistentMPIWorkerHandler.h"
#include "AbstractWorkerHandler.h"

// Initialize static child communicator of PersistentMPIWorkerHandler to the
// null communicators (MPI_COMM_NULL)
MPI_Comm PersistentMPIWorkerHandler::m_child_comm = MPI_COMM_NULL;

PersistentMPIWorkerHandler::PersistentMPIWorkerHandler(const cmd_t& command,
        const std::string& input_string) :
    AbstractWorkerHandler(command, input_string)
{
    // Spawn persistent MPI child process if it has not yet been spawned
    if (m_child_comm == MPI_COMM_NULL)
        m_child_comm = spawn_worker(m_command);

    // Write input string to spawned MPI process
    MPI_Send(input_string.c_str(), input_string.size() + 1, MPI_CHAR,
            WORKER_RANK, MANAGER_MSG_TAG, m_child_comm);
}

PersistentMPIWorkerHandler::~PersistentMPIWorkerHandler()
{
    // Just discard results, do not terminate persistent MPI child process
    discardResults();
}

void PersistentMPIWorkerHandler::terminate()
{
    // Discard results from persistent MPI child process
    discardResults();

    // Send termination signal
    int signal = TERMINATE_WORKER_SIGNAL;
    MPI_Send(&signal, 1, MPI_INT, WORKER_RANK, MANAGER_SIGNAL_TAG, m_child_comm);

    // Free communicator
    MPI_Comm_disconnect(&m_child_comm);
}

bool PersistentMPIWorkerHandler::isDone()
{
    // Probe for result if result has not yet been received
    if (    !m_result_received &&
            iprobe_wrapper(WORKER_RANK, WORKER_MSG_TAG, m_child_comm))
    {
        // Receive message
        m_output_buffer.assign(receiveMessage());

        // Receive error code
        m_error_code = receiveErrorCode();

        // Set flag
        m_result_received = true;
    }

    return m_result_received;
}

std::string PersistentMPIWorkerHandler::receiveMessage() const
{
    return receive_string(m_child_comm, WORKER_RANK, WORKER_MSG_TAG);
}

int PersistentMPIWorkerHandler::receiveErrorCode() const
{
    return receive_integer(m_child_comm, WORKER_RANK, WORKER_ERROR_CODE_TAG);
}

void PersistentMPIWorkerHandler::discardResults()
{
    // MPI does not provide process control, so
    // we can only wait for the simulation to finish
    // if it has not finished yet
    if (!m_result_received)
    {
        // Timeout if message is not ready yet
        while (!iprobe_wrapper(WORKER_RANK, WORKER_MSG_TAG, m_child_comm))
            std::this_thread::sleep_for(MAIN_TIMEOUT);

        // Receive message
        receiveMessage();

        // Receive error code
        receiveErrorCode();

        // Set flag
        m_result_received = true;
    }
}

void PersistentMPIWorkerHandler::terminatePersistent()
{
    // If this function is called, the persistent Worker must be in an idle
    // state, so it is not necessary to discard results from Worker.

    // If m_child_comm is the null communicator, the persistent Worker has
    // already been terminated, so nothing needs to be done.
    if (m_child_comm == MPI_COMM_NULL)
        return;

    // Else, send termination signal to persistent Worker
    int signal = TERMINATE_WORKER_SIGNAL;
    MPI_Send(&signal, 1, MPI_INT, WORKER_RANK, MANAGER_SIGNAL_TAG, m_child_comm);

    // Free communicator
    MPI_Comm_disconnect(&m_child_comm);
}
