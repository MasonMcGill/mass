#include "app.h"

Error current_error = ERROR_NONE;

void error_throw(Error error) {
  if (current_error == ERROR_NONE)
    current_error = error;
}

Error error_catch() {
  Error error = current_error;
  current_error = ERROR_NONE;
  return error;
}

char const *error_get_message(Error error) {
  switch (error) {
  case ERROR_NONE:
    return "No error.";
  case ERROR_NO_CAMERA:
    return "No camera is available.";
  }
}
