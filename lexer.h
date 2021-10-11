#pragma once
#include "test_runner.h"
#include <iosfwd>
#include <string>
#include <sstream>
#include <variant>
#include <stdexcept>
#include <optional>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <cctype>

namespace Parse {

namespace TokenType {
  struct Number { int value; };
  struct Id { std::string value; };
  struct Char { char value; };
  struct String { std::string value;};
  struct Class{};
  struct Return{};
  struct If {};
  struct Else {};
  struct Def {};
  struct Newline {};
  struct Print {};
  struct Indent {};
  struct Dedent {};
  struct Eof {};
  struct And {};
  struct Or {};
  struct Not {};
  struct Eq {};
  struct NotEq {};
  struct LessOrEq {};
  struct GreaterOrEq {};
  struct None {};
  struct True {};
  struct False {};
}

using TokenBase = std::variant<
  TokenType::Number, //0
  TokenType::Id, //1
  TokenType::Char, //2
  TokenType::String, //3
  TokenType::Class, //4
  TokenType::Return, //5
  TokenType::If, //6
  TokenType::Else,//7
  TokenType::Def,//8
  TokenType::Newline,//9
  TokenType::Print,//10
  TokenType::Indent,//11
  TokenType::Dedent,//12
  TokenType::And,//13
  TokenType::Or,//14
  TokenType::Not,//15
  TokenType::Eq,//16
  TokenType::NotEq,//17
  TokenType::LessOrEq,//18
  TokenType::GreaterOrEq,//19
  TokenType::None,//20
  TokenType::True,//21
  TokenType::False,//22
  TokenType::Eof//23
>;

struct Token : TokenBase {
  using TokenBase::TokenBase;

  template <typename T>
  bool Is() const { return std::holds_alternative<T>(*this); }

  template <typename T>
  const T& As() const { return std::get<T>(*this); }

  template <typename T>
  const T* TryAs() const { return std::get_if<T>(this); }
};

bool operator == (const Token& lhs, const Token& rhs);
std::ostream& operator << (std::ostream& os, const Token& rhs);

class LexerError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};


class Lexer {
public:
  explicit Lexer(std::istream& input);
  
  const Token& CurrentToken() const;
  Token NextToken();
  
  template <typename T>
  const T& Expect() const {
    if (!current_token.Is<T>()) {
      throw LexerError("");
    };
    return current_token.As<T>();
  }
  template <typename T, typename U>
  void Expect(const U& value) const {
    Expect<T>();
    auto token = current_token.As<T>();
    if (token.value != value) {
      throw LexerError("");
    }
  }
  
  template <typename T>
  const T& ExpectNext() {
    NextToken();
    return Expect<T>();
  }
  
  template <typename T, typename U>
  void ExpectNext(const U& value) {
    NextToken();
    Expect<T>(value);
  }
  
  Token CreateNumberToken(const std::string& text);
  Token CreateToken(const std::string& text);
  Token GetKeywordToken(const std::string& text);
  Token CreateTokenFromPunct(std::istream& stream);
  bool IsKeyword(const std::string& str) const;

  std::string CollectDigits(std::istream& stream);
  std::string CollectString(std::istream& stream);
  int CountNumberOfSpaces(std::istream& stream);
  Token ReadStream();

  Token HandleEof(std::istream& stream);
  Token HandleNewline(std::istream& stream);

private:
  std::istream& stream;
  Token current_token{TokenType::Eof()};
  int current_indent = 0;
  int count_spaces = 0;
  using TokenMapping = std::unordered_map<std::string, Token>;
  TokenMapping keywords = {{"class", Token(TokenType::Class())},
                          {"return", Token(TokenType::Return())},
                          {"if", Token(TokenType::If())},
                          {"else", Token(TokenType::Else())},
                          {"def", Token(TokenType::Def())},
                          {"print", Token(TokenType::Print())},
                          {"or", Token(TokenType::Or())},
                          {"None", Token(TokenType::None())},
                          {"and", Token(TokenType::And())},
                          {"True", Token(TokenType::True())},
                          {"False", Token(TokenType::False())},
                          {"not", Token(TokenType::Not())},
                          {"==", Token(TokenType::Eq())},
                          {"!=", Token(TokenType::NotEq())},
                          {">=", Token(TokenType::GreaterOrEq())},
                          {"<=", Token(TokenType::LessOrEq())},
                          {"\n", Token(TokenType::Newline())}
                          };
};

void RunLexerTests(TestRunner& test_runner);

}

