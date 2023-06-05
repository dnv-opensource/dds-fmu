cmake_minimum_required(VERSION 3.1)

file(STRINGS "${GUID}" FMU_UUID)
configure_file("${INPUT_MODEL_DESCRIPTION}" "${OUTPUT_MODEL_DESCRIPTION}" @ONLY)
