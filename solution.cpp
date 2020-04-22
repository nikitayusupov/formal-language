#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <cassert>
#include <set>
#include <cstdio>

using std::cin;
using std::cout;
using std::endl;
using std::max;
using std::string;

typedef unsigned long ulong;

const char EPSILON = '1';

struct ParseException : public std::logic_error {
    ParseException(const string & message) : std::logic_error(message) {}
};

enum OperatorType {
    PLUS,
    MULTIPLY,
    KLEENE_STAR
};

// Структура, описывающая язык L(Operand), соответствующий
// какому-то регулярному выражению, который является
// операндом исходного регулярного выражения
struct Operand {
private:

    std::vector<std::vector<int> > containsSubstring;
    // containsSubstring[i][j] == true <=> подслово (данного слова) длины j,
    // начинающееся в i-ой позиции, содержится в нашем языке L(Operand)

    bool containsEpsilon;
    // containsEpsilon == true <=> пустое слово принадлежит нашему языку L(Operand)

    bool containsWordAsSubstring;
    // containsWordAsSubstring == true <=> данное слово word содержится
    // в качестве подслова какого-либо слова v из языка L(Operand)

    std::vector<int> containsSuffixEqualsToPrefix;
    // containsSuffixEqualsToPrefix[length] == true <=> есть в языке L(Operand) слово v, такое,
    // что суффикс u слова v является префиксом длины length данного слова word

    std::vector<int> containsPrefixEqualsToSuffix;
    // containsPrefixEqualsToSuffix[length] == true <=> есть в языке L(Operand) cлово v, такое,
    // что префикс u слова v является суффиксом длины length данного слова word

    ulong wordLength;
    // длина данного слова word

    void updateContainsWordAsSubstringForMultiply(Operand &result, const Operand &left, const Operand &right) const {
        result.containsWordAsSubstring = left.containsWordAsSubstring || right.containsWordAsSubstring;

        // L1 ~ left; L2 ~ right; L1.L2 ~ result

        // word = CCCCCTTTT
        //        ^^^^^      - prefix
        //             ^^^^  - suffix

        // L1: XXXXXCCCCC             L2: TTTTYYYYYY    ->   L1.L2:   XXXXXCCCCCTTTTYYYYYY
        //          ^^^^^                 ^^^^                             ^^^^^^^^^
        //       suffix equals        prefix equals             word contains as substring in new language
        //         to prefix            to suffix

        for (ulong prefixLength = 1; prefixLength < wordLength; ++prefixLength) {
            ulong suffixLength = wordLength - prefixLength;
            assert(suffixLength > 0 && suffixLength < wordLength);

            result.containsWordAsSubstring |=
                    left.containsSuffixEqualsToPrefix[prefixLength]
                    && right.containsPrefixEqualsToSuffix[suffixLength];
        }

        // word == CCCCTTTTYYY
        //         ^^^^^^^^    - prefix with prefixLength
        //         ^^^^        - subPrefix with subPrefixLength
        //             ^^^^    - suffixOfPrefix with suffixOfPrefixLength

        // L1: XXCCCC  L2: TTTT            ->      L1.L2:  XXCCCCTTTT
        //       ^^^^      ^^^^                              ^^^^^^^^
        //    subPrefix   suffixOfPrefix               new suffix equals to prefix

        for (ulong prefixLength = 1; prefixLength < wordLength; ++prefixLength) {
            result.containsSuffixEqualsToPrefix[prefixLength] = right.containsSuffixEqualsToPrefix[prefixLength];
            result.containsSuffixEqualsToPrefix[prefixLength] |=
                    left.containsSuffixEqualsToPrefix[prefixLength] && right.containsEpsilon;

            for (ulong subPrefixLength = 1; subPrefixLength < prefixLength; ++subPrefixLength) {
                ulong suffixOfPrefixLength = prefixLength - subPrefixLength;

                result.containsSuffixEqualsToPrefix[prefixLength] |=
                        right.containsSubstring[subPrefixLength][suffixOfPrefixLength]
                        && left.containsSuffixEqualsToPrefix[subPrefixLength];
            }

        }

        // word == YYYYCCCCCCTTTT
        //             ^^^^^^^^^^  - suffix
        //                   ^^^^  - subSuffix
        //             ^^^^^^      - prefix of suffix

        // L1: CCCCCC         L2: TTTTXXXX       ->    L1.L2:   CCCCCCTTTTXXXX
        //     ^^^^^^             ^^^^                          ^^^^^^^^^^
        //  prefix of suffix     subSuffix                   new prefix equals to suffix

        for (ulong suffixLength = 1; suffixLength < wordLength; ++suffixLength) {
            result.containsPrefixEqualsToSuffix[suffixLength] = left.containsPrefixEqualsToSuffix[suffixLength];
            result.containsPrefixEqualsToSuffix[suffixLength] |=
                    left.containsEpsilon && right.containsPrefixEqualsToSuffix[suffixLength];

            for (ulong subSuffixLength = 1; subSuffixLength < suffixLength; ++subSuffixLength) {
                ulong prefixOfSuffixLength = suffixLength - subSuffixLength;

                result.containsPrefixEqualsToSuffix[suffixLength] |=
                        left.containsSubstring[wordLength - suffixLength][prefixOfSuffixLength]
                        && right.containsPrefixEqualsToSuffix[subSuffixLength];
            }
        }

    }

    void updateContainsSubstringForMultiply(Operand &result, const Operand &left, const Operand &right) const {
        for (ulong startPosition = 0; startPosition < wordLength; ++startPosition) {
            for (ulong length = 1; length <= wordLength - startPosition; ++length) {
                for (ulong prefixLength = 0; prefixLength <= length; ++prefixLength) {

                    ulong suffixLength = length - prefixLength;

                    prefixLength++;
                    prefixLength--;

                    if (prefixLength == 0) {
                        if (left.containsEpsilon) {
                            result.containsSubstring[startPosition][length] |=
                                    right.containsSubstring[startPosition][length];
                        }
                        continue;
                    }

                    if (suffixLength == 0) {
                        if (right.containsEpsilon) {
                            result.containsSubstring[startPosition][length] |=
                                    left.containsSubstring[startPosition][length];
                        }
                        continue;
                    }

                    result.containsSubstring[startPosition][length] |=
                            left.containsSubstring[startPosition][prefixLength] &&
                            right.containsSubstring[startPosition + prefixLength][suffixLength];


                    // Слово W длины length лежит в языке (L1 . L2), если найдется
                    // такой префикс длины sublength слова W, что этот префикс
                    // принадлежит языку L1, и при этом языку L2 принадлежит
                    // суффикс длины (length - sublegth) слова W

                }
            }
        }

        result.containsEpsilon = left.containsEpsilon && right.containsEpsilon;
    }

public:

    // Операнд, задающий язык из одного символа
    Operand(char character, const string &word) {
        containsSubstring = std::vector<std::vector<int> >(word.length() + 1, std::vector<int>(word.length() + 1, 0));
        containsPrefixEqualsToSuffix.clear();
        containsPrefixEqualsToSuffix.resize(word.length() + 1);
        containsSuffixEqualsToPrefix.clear();
        containsSuffixEqualsToPrefix.resize(word.length() + 1);
        wordLength = word.length();

        if (character == EPSILON) {
            containsEpsilon = true;
            containsWordAsSubstring = false;
        } else {
            containsEpsilon = false;

            if (word.length() == 1) {
                containsWordAsSubstring = word[0] == character;
            } else {
                containsWordAsSubstring = false;
            }

            if (word[0] == character) {
                containsSuffixEqualsToPrefix[1] = true;
            }

            if (word.back() == character) {
                containsPrefixEqualsToSuffix[1] = true;
            }

            for (ulong startPosition = 0; startPosition < wordLength; ++startPosition) {
                if (character == word[startPosition]) {
                    containsSubstring[startPosition][1] = true;
                }
            }
        }
    }

    // Операнд, задающий пустой язык
    Operand(ulong wordLength) : containsSubstring(wordLength + 1, std::vector<int>(wordLength + 1, 0)),
                                containsEpsilon(false), containsWordAsSubstring(false),
                                containsSuffixEqualsToPrefix(wordLength + 1),
                                containsPrefixEqualsToSuffix(wordLength + 1),
                                wordLength(wordLength) {}

    Operand() {}

    bool isWordEqualToSomeSubstringInLanguage() const {
        return containsWordAsSubstring;
    }

    Operand operator+(const Operand &right) const {
        Operand left = *this;

        for (ulong startPosition = 0; startPosition < wordLength; ++startPosition) {
            for (ulong length = 0; length <= wordLength - startPosition; ++length) {
                left.containsSubstring[startPosition][length] |= right.containsSubstring[startPosition][length];
            }
        }

        for (ulong length = 1; length <= wordLength; ++length) {
            left.containsSuffixEqualsToPrefix[length] |= right.containsSuffixEqualsToPrefix[length];
            left.containsPrefixEqualsToSuffix[length] |= right.containsPrefixEqualsToSuffix[length];
        }

        left.containsEpsilon |= right.containsEpsilon;
        left.containsWordAsSubstring |= right.containsWordAsSubstring;

        assert(left.wordLength == right.wordLength);

        return left;
    }

    Operand operator*(const Operand &right) const {
        Operand left = *this;
        Operand result(wordLength);

        updateContainsSubstringForMultiply(result, left, right);

        updateContainsWordAsSubstringForMultiply(result, left, right);

        return result;
    }

};

struct Expression {
private:
    std::stack<Operand> operands;
    string expression;

    bool isOperator(char character) const {
        std::set<char> allOperators({'+', '.', '*'});
        return allOperators.count(character) == 1;
    }

    bool isSymbolOfAlphabet(char character) const {
        std::set<char> allSymbols({'a', 'b', 'c', EPSILON});
        return allSymbols.count(character) == 1;
    }

    void checkWord(const string &word) const {
        if (word.empty()) {
            throw ParseException("Word is empty");
        }
        for (char element : word) {
            if (element == EPSILON || !isSymbolOfAlphabet(element)) {
                string message = "Unknown symbol in word: " + string(1, element);
                throw ParseException(message);
            }
        }
    }

    OperatorType operatorCode(char character) const {
        switch (character) {
            case '+':
                return PLUS;
            case '.':
                return MULTIPLY;
            case '*':
                return KLEENE_STAR;
            default:
                string message = "Unknown operator symbol: " + string(1, character);
                throw ParseException(message);
        }
    }

    void calculateOperator(const string &word, OperatorType currentOperator) {
        // Calculate PLUS or MULTIPLY or KLEENE STAR
        if (currentOperator == KLEENE_STAR) {
            calculateKleeneStar(word);
            return;
        }

        // Calculate PLUS or MULTIPLY
        if (operands.size() < 2) {
            throw ParseException("Missing operands");
        }

        Operand right = operands.top();
        operands.pop();
        Operand left = operands.top();
        operands.pop();

        operands.push(currentOperator == PLUS ? left + right : left * right);
    }

    void calculateKleeneStar(const string &word) {
        if (operands.size() < 1) {
            throw ParseException("Missing operands");
        }

        // e* = e^0 + e^1 + e^2 ... e^n + e^(n+1) ...

        // n == 0:
        Operand currentPow(EPSILON, word); // Операнд, задающий язык из пустого слова
        Operand startOperand = operands.top(); // startOperand := e
        operands.pop();
        Operand currentOperand = currentPow; // e^0 -- язык из пустого слова

        // n > 0 && n < 2 * length + 2:
        for (ulong i = 0; i < 2 * word.length() + 2; ++i) {
            Operand nextPow = currentPow * startOperand; // nextPow := e^n * e
            Operand nextOperand = currentOperand + nextPow; // nextOperand := e^n + e^(n+1)
            currentPow = nextPow;
            currentOperand = nextOperand;
        }

        operands.push(currentOperand);
    }

public:

    void readExpression() {
        cin >> expression;

        if (expression.empty()) {
            throw ParseException("Expression is empty");
        }
    }

    Operand calculateValueOfExpression(const string &word) {
        checkWord(word);

        for (ulong i = 0; i < expression.length(); ++i) {
            if (isOperator(expression[i])) {
                OperatorType currentOperator = operatorCode(expression[i]);
                calculateOperator(word, currentOperator);
            } else if (isSymbolOfAlphabet(expression[i])) {
                operands.push(Operand(expression[i], word));
            } else {
                string message = "Unknown symbol in expression: " + string(1, expression[i]);
                throw ParseException(message);
            }
        }

        if (operands.size() > 1) {
            throw ParseException("Too much operands");
        }
        if (operands.size() < 1) {
            throw ParseException("Missing operands");
        }
        return operands.top();
    }

};

struct Solver {
private:
    Expression expression;
    string word;

public:
    Solver(const Expression &expression, const string &word) :
            expression(expression), word(word) {}

    Solver() {}

    ulong solve() {
        ulong answer = 0;

        for (ulong startPosition = 0; startPosition < word.length(); ++startPosition) {
            for (ulong length = 1; length <= word.length() - startPosition; ++length) {
                string toCheck = word.substr(startPosition, length);

                Expression bufferExpression = expression;
                Operand result = bufferExpression.calculateValueOfExpression(toCheck);

                if (result.isWordEqualToSomeSubstringInLanguage()) {
                    answer = max(answer, length);
                }
            }
        }

        return answer;
    }
};

int main() {
    freopen("input.txt", "rt", stdin);

    Expression expression;
    string word;

    try {
        expression.readExpression();
        cin >> word;

        Solver solver(expression, word);

        cout << solver.solve() << endl;
    } catch (ParseException e) {
        std::cerr << e.what() << endl;
        return 1;
    }

    return 0;
}