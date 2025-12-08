#include "l1parser.hpp"
#include <iostream>
#include <vector>
#include <string>

void usage() {
    std::cerr << "Usage: l1util list | get <dev> <prop> | idx2if <idx> | if2zone <ifname> | if2dat <ifname> | zone2if <zone> | if2dbdcidx <ifname>" << std::endl;
    exit(1);
}

void print_result(std::optional<std::string> res) {
    if (res.has_value()) {
        std::cout << res.value() << std::endl;
    } else {
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    L1Parser parser;
    if (!parser.load(L1_DAT_PATH)) {
        std::cerr << "Error: Failed to load profile: " << L1_DAT_PATH << std::endl;
        return 1;
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    if (args.empty()) usage();

    const std::string& cmd = args[0];

    if (cmd == "list") {
        std::cout << parser.list_devs() << std::endl;
    } 
    else if (cmd == "get") {
        if (args.size() != 3) usage();
        print_result(parser.get_prop(args[1], args[2]));
    }
    else if (cmd == "if2zone") {
        if (args.size() != 2) usage();
        print_result(parser.if2zone(args[1]));
    }
    else if (cmd == "if2dat") {
        if (args.size() != 2) usage();
        print_result(parser.if2dat(args[1]));
    }
    else if (cmd == "zone2if") {
        if (args.size() != 2) usage();
        print_result(parser.zone2if(args[1]));
    }
    else if (cmd == "if2dbdcidx") {
        if (args.size() != 2) usage();
        print_result(parser.if2dbdcidx(args[1]));
    }
    else if (cmd == "idx2if") {
        if (args.size() != 2) usage();
        try {
            int idx = std::stoi(args[1]);
            print_result(parser.idx2if(idx));
        } catch (...) {
            std::cerr << "Error: Invalid index number" << std::endl;
            return 1;
        }
    }
    else {
        usage();
    }

    return 0;
}