#ifndef LOG_HPP
#define LOG_HPP

#ifdef LOG
  template<typename T>
  void _log_impl(T msg, char *filename, int lineno) {
    Serial.print(filename);
    Serial.print(" L");
    Serial.print(lineno);
    Serial.print(": ");
    Serial.println(msg);
  }
  #define logmsg(msg) _log_impl(msg, __FILE__, __LINE__)
#else
  #define logmsg(msg)
#endif  //  LOG

#endif  //  LOG_HPP
