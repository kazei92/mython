#include "lexer.h"
#include <algorithm>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace Parse {

bool operator == (const Token& lhs, const Token& rhs) {
  using namespace TokenType;

  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.Is<Char>()) {
    return lhs.As<Char>().value == rhs.As<Char>().value;
  } else if (lhs.Is<Number>()) {
    return lhs.As<Number>().value == rhs.As<Number>().value;
  } else if (lhs.Is<String>()) {
    return lhs.As<String>().value == rhs.As<String>().value;
  } else if (lhs.Is<Id>()) {
    return lhs.As<Id>().value == rhs.As<Id>().value;
  } else {
    return true;
  }
}

std::ostream& operator << (std::ostream& os, const Token& rhs) {
  using namespace TokenType;

#define VALUED_OUTPUT(type) \
  if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

  VALUED_OUTPUT(Number);
  VALUED_OUTPUT(Id);
  VALUED_OUTPUT(String);
  VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

  UNVALUED_OUTPUT(Class);
  UNVALUED_OUTPUT(Return);
  UNVALUED_OUTPUT(If);
  UNVALUED_OUTPUT(Else);
  UNVALUED_OUTPUT(Def);
  UNVALUED_OUTPUT(Newline);
  UNVALUED_OUTPUT(Print);
  UNVALUED_OUTPUT(Indent);
  UNVALUED_OUTPUT(Dedent);
  UNVALUED_OUTPUT(And);
  UNVALUED_OUTPUT(Or);
  UNVALUED_OUTPUT(Not);
  UNVALUED_OUTPUT(Eq);
  UNVALUED_OUTPUT(NotEq);
  UNVALUED_OUTPUT(LessOrEq);
  UNVALUED_OUTPUT(GreaterOrEq);
  UNVALUED_OUTPUT(None);
  UNVALUED_OUTPUT(True);
  UNVALUED_OUTPUT(False);
  UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

  return os << "Unknown token :(";
}

Lexer::Lexer(std::istream& input) : stream(input) { current_token = NextToken(); };
const Token& Lexer::CurrentToken() const { return current_token; }

Token Lexer::NextToken() {
  current_token = ReadStream();
  return current_token;
}

string Lexer::CollectDigits(istream& stream) {
  string digits;
  while (isdigit(stream.peek())) {
    digits.push_back(stream.get());
  }
  return digits;
}

string Lexer::CollectString(istream& stream) {
  string str;
  while (isalnum(stream.peek()) || stream.peek() == '_') {
    str.push_back(stream.get());
  }
  return str;
}

int Lexer::CountNumberOfSpaces(std::istream& stream) {
  int count = 0;
  while (isblank(stream.peek())) {
    stream.get();
    ++count;
  }
  return count;
};

bool Lexer::IsKeyword(const std::string& str) const {
  auto search = keywords.find(str);
  return search != keywords.end();
}

Token Lexer::GetKeywordToken(const std::string& str) {
  return keywords[str];
}

Token Lexer::CreateTokenFromPunct(std::istream& stream) {
  char c = stream.get();
    
  if ((c == '!' || c == '=' || c == '>' || c == '<') && (stream.peek() == '=')) {
    char n = stream.get();
    return GetKeywordToken(string({c, n}));
  }
  
  if (c == '\"' || c == '\'') {
    string str;
    while (stream.peek() != c) { str.push_back(stream.get()); }
    stream.get();
    return Token(TokenType::String{str});
  }

  if (isalnum(c) || c == '_') {
    string str({c});
    return Token(TokenType::Id{c+CollectString(stream)});
  }
  
  return Token(TokenType::Char{c});
}

Token Lexer::HandleEof(std::istream& stream) {
  if (current_indent > 0) {
    current_indent -= 2;
    return Token(TokenType::Dedent{});
  }
  
  if (current_token == Token(TokenType::Newline{})) { return Token(TokenType::Eof{}); }
  
  if (current_token == Token(TokenType::Eof{})) { return Token(TokenType::Eof{});}
  
  if (!(current_token == Token(TokenType::Newline{}))) {
    stream.clear();
    stream.unget();
    char c = stream.get();
    if (isalnum(c) || ispunct(c)) { return Token(TokenType::Newline{}); }
  }
  return Token(TokenType::Eof{}); 
}

Token Lexer::HandleNewline(std::istream& stream) {
  stream.get();
  if (current_token == Token(TokenType::Newline{}) || current_token == Token(TokenType::Eof{})) {
    return ReadStream();
  }
  return Token(TokenType::Newline{});
}

Token Lexer::CreateNumberToken(const std::string& text) {
  int n = stoi(text);
  return Token(TokenType::Number{n});
}

Token Lexer::ReadStream() {

  if (stream.peek() == EOF) { return HandleEof(stream); }

  if (count_spaces != 0) {
    if (count_spaces > 0) {
      current_indent += 2;
      count_spaces -= 2;
      return Token({Parse::TokenType::Indent()});
    }
    else {
      current_indent -= 2;
      count_spaces += 2;
      return Token({Parse::TokenType::Dedent()});
    }
    
  }

  if (current_token == Token(TokenType::Newline{})) {
    while (stream.peek() == '\n') {
      stream.get();
  }

    int n_of_spaces = CountNumberOfSpaces(stream);
    if (n_of_spaces > current_indent) {
      current_indent += 2;
      count_spaces = n_of_spaces - current_indent;
      return Token({Parse::TokenType::Indent()});
    }
    else if (n_of_spaces < current_indent) {
      current_indent -= 2;
      count_spaces = n_of_spaces - current_indent;
      return Token({Parse::TokenType::Dedent()});
    }
    current_token = Token(TokenType::Eof{});
    return ReadStream();
  }

  if (stream.peek() == '\n') { return HandleNewline(stream); }
  if (ispunct(stream.peek())) { return CreateTokenFromPunct(stream); }

  if (isdigit(stream.peek())) {
    string digits = CollectDigits(stream);
    return CreateNumberToken(digits);
  }

  if (isalnum(stream.peek()) || stream.peek() == '_') {
    string str = CollectString(stream);
    if (IsKeyword(str)) { return GetKeywordToken(str);}
    return Token(TokenType::Id{str});
  }

  if (isblank(stream.peek())) {
    int n_of_spaces = CountNumberOfSpaces(stream); 
    return ReadStream();
  }
  return Token(TokenType::Eof{});
}

} /* namespace Parse */