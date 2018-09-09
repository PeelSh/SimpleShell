//
// Created by Liam Heeger on 8/30/18.
//

#include "cd_command.h"

#include <cstdlib>
#include <unistd.h>

using namespace shell;

CdCommand::
CdCommand(const Command& command)
        : InternalCommand(command) {}

void
CdCommand::
Run(ShellInfo* shell_info, std::string* output) {
    int argument_count = command_.sub_command(0).argument_size();
    int change_dir_val;

    // TODO need to check if is a valid path.
    if (argument_count == 0) {
        // TODO handle syscall error.
        change_dir_val = chdir(std::getenv("HOME"));
    } else {
        change_dir_val = chdir(command_.sub_command(0).argument(0).c_str());
    }

    char current_working_directory[PATH_MAX];
    std::string working_directory(getcwd(current_working_directory, sizeof(current_working_directory)));

    shell_info->set_working_directory(working_directory);
}

