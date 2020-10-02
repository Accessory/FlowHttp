#pragma once

enum class HttpParserState {
    START,
    CONTENT_START,
    CONTENT_HEADER,
    CONTENT_HEADERFIELD,
    CONTENT_DATA,
    END,
    BAD_REQUEST
};