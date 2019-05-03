
#ifndef FEASST_CORE_ARGUMENTS_H_
#define FEASST_CORE_ARGUMENTS_H_

#include <vector>
#include <iostream>
#include <map>
#include <string>

namespace feasst {

/// Use a map of string pairs to function as a dictionary for arguments.
typedef std::map<std::string, std::string> argtype;
typedef std::map<std::string, argtype> arglist;

/**
 * The Arguments class takes in arguments as a map of strings
 * (e.g., key,value pairs in a dictionary).
 * It then uses chainsetters to access the key values and finds keys that
 * were used for error checking.
 */
class Arguments {
 public:
  void init(const argtype &args);

  /**
   * Set the argument key (the first in the map/dictionary pair).
   * Store the key for error processing.
   * Return self for chainsetting.
   */
  Arguments& key(const std::string &key);

  /// Set the default value if key is not present in args.
  Arguments& dflt(const std::string &defaultVal);

  /// Return true if key is not present in args. Otherwise, false.
  bool empty();

  /// Return true if the key is used (e.g., the inverse of empty()).
  bool used() { return !empty(); }

  /// Return the value of the processed keyword.
  /// Reset key and dflt
  std::string str();

  /// Return the conversion of a str of the processed keyword to double
  /// a precision floating point number.
  double dble();

  /// Return the conversion of a str of the processed keyword to int.
  int integer();

  /// Return the conversion of a str of the processed keyword to boolean.
  /// Accept the strings "true", "false", "1" or "0".
  bool boolean();

  /// Upon destruction, check that all provided args were processed.
  /// Automatically include empty string key as processed.
  bool check_all_used();

  /// Chainset argparse to remove the next arg returned by str from args
  Arguments& remove();

  /// Return the size of the args (e.g., the number)
  int size() const { return static_cast<int>(args_.size()); }

  argtype args() const { return args_; }  //!< Return args

  /// Print the status of the arguments to human readable string.
  std::string status() const;

  ~Arguments() { check_all_used(); }

 private:
  argtype args_;
  std::string key_;
  std::string default_value_;
  std::vector<std::string> used_keys_;
  bool remove_ = false;
};

}  // namespace feasst

#endif  // FEASST_CORE_ARGUMENTS_H_
