file(STRINGS ${DATA_FILE} DATA_CONTENTS)
file(READ ${IN_FILE} FILE_CONTENTS)
list(GET DATA_CONTENTS 0 MATCH_STRING)
list(GET DATA_CONTENTS 1 REPLACE_STRING)
string(REPLACE "${MATCH_STRING}" "${REPLACE_STRING}" OUT_CONTENTS "${FILE_CONTENTS}")
file(WRITE ${OUT_FILE} "${OUT_CONTENTS}")
