#pragma once

#include <FlowUtils/FlowArgParser.h>

class ArgParserUtil {
public:
    static FlowArgParser defaultArgParser() {
        FlowArgParser fap;
        fap.addIndexOption("address", "Server listening address");
        fap.addIndexOption("port", "Server listening port");
        fap.addIndexOption("path", "Server path");
        fap.addParameterOption("t", "threads", "Max Request Threads");
        fap.addParameterOption("k", "key", "PrivateKey");
        fap.addParameterOption("c", "cert", "Certificate");
        fap.addParameterOption("d", "dh", "Diffie Hellman file");
        return fap;
    }
};