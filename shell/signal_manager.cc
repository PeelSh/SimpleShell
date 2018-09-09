//
// Created by Liam Heeger on 9/1/18.
//

#include "signal_manager.h"

#include <csignal>
#include <signal.h>
#include <sys/types.h>

using namespace shell;

namespace {

// TODO move to ShellInfoService
bool
GetForegroundJob(std::shared_ptr<ShellInfo> shell_info, Job* fg_job) {
    for (const Job& job : shell_info->job()) {
        if (job.status() == Job_Status_FOREGROUND) {
            fg_job->CopyFrom(job);
            return true;
        }
    }

    return false;
}

bool
SetForegroundJobStatus(std::shared_ptr<ShellInfo> shell_info, const Job_Status& status) {
    for (auto job_iter = shell_info->mutable_job()->pointer_begin();
         job_iter !=  shell_info->mutable_job()->pointer_end();
         job_iter++) {
        if ((*job_iter)->status() == Job_Status_FOREGROUND) {
            (*job_iter)->set_status(status);
            return true;
        }
    }

    return false;
}

bool
SetJobStatus(std::shared_ptr<ShellInfo> shell_info, int job_id, const Job_Status& status) {
    for (auto job_iter = shell_info->mutable_job()->pointer_begin();
         job_iter !=  shell_info->mutable_job()->pointer_end();
         job_iter++) {
        if ((*job_iter)->job_id() == job_id) {
            (*job_iter)->set_status(status);
            return true;
        }
    }

    return false;
}

// TODO move to ShellInfoService
bool
GetJobIdByPid(std::shared_ptr<ShellInfo> shell_info, pid_t pid, int* job_id) {
    for (const Job& job : shell_info->job()) {
        for (int process_id : job.process_id()) {
            if (process_id == pid) {
                *job_id = job.job_id();
                return true;
            }
        }
    }

    return false;
}

}

SignalManager* SignalManager::instance = nullptr;

SignalManager::
SignalManager() {}

SignalManager*
SignalManager::
GetInstance() {
    if (instance == nullptr) {
        instance = new SignalManager();
    }

    return instance;
}

bool
SignalManager::
Start() {
    // TODO handler sys call errors for signal mask/signal handlers.
    // Set up signal mask.
    sigemptyset(&signal_mask_);
    sigaddset(&signal_mask_, SIGINT);
    sigaddset(&signal_mask_, SIGTSTP);
    sigaddset(&signal_mask_, SIGCHLD);

    // Set up signal handlers.
    std::signal(SIGINT, &SignalManager::SigIntHandler);
    std::signal(SIGTSTP, &SignalManager::SigTstpHandler);
    std::signal(SIGCHLD, &SignalManager::SigChldHandler);

    return true;
}

bool
SignalManager::
Stop() {
    // TODO reset signal handlers to default.
    return true;
}

void
SignalManager::
SetShellInfo(std::shared_ptr<ShellInfo> shell_info) {
    shell_info_ = shell_info;
}

bool
SignalManager::
BlockSignals() {
    // TODO handle syscall errors and amend return value to match error state.
    sigprocmask(SIG_BLOCK, &signal_mask_, nullptr);
    return true;
}

bool
SignalManager::
UnblockSignals() {
    // TODO handle syscall errors and amend return value to match error state.
    sigprocmask(SIG_UNBLOCK, &signal_mask_, nullptr);
    return true;
}

void
SignalManager::
SigIntHandler(int signal) {
    Job fg_job;
    if (GetForegroundJob(instance->shell_info_, &fg_job)) {
        // TODO handle sys call error
        kill(-(fg_job.process_group_id()), SIGINT);
    } // TODO handle no fg job found.
}

void
SignalManager::
SigTstpHandler(int signal) {
    Job fg_job;
    std::cout << "TSTP" << std::endl;
    if (GetForegroundJob(instance->shell_info_, &fg_job)) {
        // TODO handle sys call error
        kill(-(fg_job.process_group_id()), SIGTSTP);
    } // TODO handle no fg job found.
}

void
SignalManager::
SigChldHandler(int signal) {
    // TODO implement.

    pid_t child_pid;
    int status;

    while ((child_pid = waitpid(-1, &status, (WNOHANG | WUNTRACED))) > 0) {
        // TODO do something with this. Possibly reap in a new JobService?
        int job_id;

        // TODO Support different wait statuses.
        if (GetJobIdByPid(instance->shell_info_, child_pid, &job_id)) {
            if (WIFSTOPPED(status)) {
                // TODO print a message about job changing.
                SetJobStatus(instance->shell_info_, job_id, Job::STOPPED);
            } else if (WIFSIGNALED(status)) {
                SetJobStatus(instance->shell_info_, job_id, Job::UNDEFINED);
            } else if (WIFEXITED(status)) {
                SetJobStatus(instance->shell_info_, job_id, Job::UNDEFINED);
            } else {
                // TODO handle error.
            }

        } else {
            // TODO handle error.
        }
    }
}
