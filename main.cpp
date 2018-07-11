#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <stack>
#include <climits>
#include <sstream>

#define int64 long long int
#define ldouble long double

// arbitrary for now
#define MAX_CONSTANT_DIGITS 9
#define MAX_CONSTANT_HEXDIGITS 10

#define MAXY_UPPER_BOUND LLONG_MAX
#define MAXY_LOWER_BOUND 382LL
#define MINY_UPPER_BOUND (-1LL*MAXY_LOWER_BOUND)
#define MINY_LOWER_BOUND (-1LL*MAXY_UPPER_BOUND)

#define MAXX_UPPER_BOUND LLONG_MAX
#define MAXX_LOWER_BOUND 260LL
#define MINX_UPPER_BOUND (-1LL*MAXX_LOWER_BOUND)
#define MINX_LOWER_BOUND (-1LL*MAXX_UPPER_BOUND)

#define VALID_CONSTANT_IND LLONG_MAX

#define NUM_ALLOWED_OPERATORS 12

#define MAX_SHIFTABLE_BITS 63LL
#define MIN_SHIFTABLE_BITS 0LL

static const char global_delim = '$';

struct displayParameters {
   ldouble vis_maxx, vis_minx;
   ldouble vis_maxy, vis_miny;
   ldouble vis_diffx, vis_diffy;
   ldouble maxx, minx;
   ldouble maxy, miny;
   int incx;
};

// refactor globals later
std::string temp_global_stry = "-((t&1ff)*(~t&1ff)>>9)*((((((t^(t<<1f))-(t<<1f))%400)/200)*2)-1)";
std::string temp_global_strx = "-(((t-ff)&1ff)*(~(t-ff)&1ff)>>9)*(((((((t-ff)^((t-ff)<<1f))-((t-ff)<<1f))%400)/200)*2)-1)";
Fl_Input *inpy;
Fl_Input *inpx;
bool parse_success = false;
// ...

static const int operator_prec[NUM_ALLOWED_OPERATORS] = {
     0,0,1,1,1,2,2,3,3,4,5,6
};

enum operator_value {
     OPERATOR_OPP,
     OPERATOR_CDP,
     OPERATOR_NOT,
     OPERATOR_NEG,
     OPERATOR_MUL,
     OPERATOR_DIV,
     OPERATOR_MOD,
     OPERATOR_ADD,
     OPERATOR_SUB,
     OPERATOR_LSF,
     OPERATOR_RSF,
     OPERATOR_AND,
     OPERATOR_XOR,
     OPERATOR_IOR
};

enum token_type {
     TOKEN_CONSTANT_OPERAND,
     TOKEN_VARIABLE_OPERAND,
     TOKEN_INFIX_OPERATOR,
     TOKEN_PREFIX_OPERATOR,
     TOKEN_PARENTHESIS_OPEN,
     TOKEN_PARENTHESIS_CLOSE
};

struct token {
     std::string value;
     token_type type;
};

bool isVariable(char c) {
     return c == 't';
}

bool isDigit(char c) {
     return (int)c >= (int)'0' &&
            (int)c <= (int)'9';
}

bool isHexDigit(char c) {
     return isDigit(c) || (c >= (int)'a' && c <= (int)'f');
}

bool isOperatorChar(char c) {
     return c == '~' || c == '*' || c == '/' || c == '%' || c == '+' || c == '-' || c == '>' || c == '<' || c == '&' || c == '^' || c == '|';
}

bool isParenthesis(char c) {
     return c == '(' || c == ')';
}

bool isPotentiallyBeforeUnaryMinus(char c) {
     return isOperatorChar(c) || c == '(';
}

bool isPrefixOperatorIndicatorChar(char c) {
     return c == '~' || c == 'u';
}

operator_value getTokenOperatorEquivalent(token tok)  {
     switch(tok.value[0]) {
         case('~'):
              return OPERATOR_NOT;
         case('u'):
              return OPERATOR_NEG;
         case('*'):
              return OPERATOR_MUL;
         case('/'):
              return OPERATOR_DIV;
         case('%'):
              return OPERATOR_MOD;
         case('+'):
              return OPERATOR_ADD;
         case('-'):
              return OPERATOR_SUB;
         case('<'):
              return OPERATOR_LSF;
         case('>'):
              return OPERATOR_RSF;
         case('&'):
              return OPERATOR_AND;
         case('^'):
              return OPERATOR_XOR;
         default:
              break;
     }

     return OPERATOR_IOR;
}

token_type getTokenType(std::string str) {
    const char c = str[0];

    if (c == '(')
        return TOKEN_PARENTHESIS_OPEN;
    if (c == ')')
        return TOKEN_PARENTHESIS_CLOSE;
    if (isHexDigit(c))
        return TOKEN_CONSTANT_OPERAND;
    if (isVariable(c))
        return TOKEN_VARIABLE_OPERAND;
    if (isPrefixOperatorIndicatorChar(c))
        return TOKEN_PREFIX_OPERATOR;

    return TOKEN_INFIX_OPERATOR;
}

// experimental
int64 getint64FromHexStr(std::string h) {
    int64 result;
    std::stringstream ss;
    ss << std::hex << h;
    ss >> result;
    return result;
}

void evaluateButton_CB(Fl_Widget *, void *);
void modifyExpressionXString_CB(Fl_Widget *, void *);
void modifyExpressionYString_CB(Fl_Widget *, void *);

class evaluator {
    public:
         evaluator();
         void init(std::string);
         bool canPerformScopeOperation(operator_value);
         void separateStringTokens(std::string);
         void simplifyExpression(int64);
         void performOperation(int64);
         void resetStacks();
         void clearValues();
         void populateParseTokens();
         void resetInvalidIndices();
         bool checkInvalidChars();
         bool checkInvalidExpression();
         bool currentExpressionBad();
         bool evaluationErrorOccurred();
         bool FPEOccurred();
         bool UDFOccurred();
         int getNumValues();
         int64 queryMinValue();
         int64 queryMaxValue();
         int64 getValue(int);
         std::string getExpressionString();
    private:
         std::stack<operator_value> operators;
         std::stack<int64> operands;
         std::vector<std::string> expression_vec;
         std::vector<char> parse_results;
         std::vector<token> tokens;
         std::vector<int64> values;
         std::string expression_str;
         bool bad_expression;
         int64 fpe_index;
         int64 udf_index;
};

class visualizer : public Fl_Box {
    public:
         visualizer(int,int,int,int);
         int getClocx();
         int getClocy();
         int getXZeroOffset();
         int getYZeroOffset();
         int getNumValues();
         int handle(int);
         bool FPEOccurred();
         bool UDFOccurred();
         bool evaluationErrorOccurred();
         bool parseErrorOccurred();
         evaluator *getXEvaluator();
         evaluator *getYEvaluator();
         void resetInvalidIndices();
         void updateMinMaxValues();
         void draw();
    private:
         evaluator Xevaluator;
         evaluator Yevaluator;
         std::string cursor_str;
         displayParameters dparams;
         int box_locx, box_locy;
         int num_values;
         bool show_tooltip;
};

visualizer::visualizer(int x,int y,int w,int h) : Fl_Box(x,y,w,h,0) {

    dparams.vis_maxx = (ldouble)w;
    dparams.vis_minx = 0.0;

    dparams.vis_maxy = (ldouble)h;
    dparams.vis_miny = 0.0;

    dparams.vis_diffx = (ldouble)w;
    dparams.vis_diffy = (ldouble)h;

    dparams.maxx = (ldouble)w/2.0;
    dparams.minx = -1.0 * dparams.maxx;

    dparams.maxy = (ldouble)h/2.0;
    dparams.miny = -1.0 * dparams.maxy;

    dparams.incx = 1;

    box_locx = x;
    box_locy = y;

    show_tooltip = false;

    cursor_str = "";

    Xevaluator.init(temp_global_strx);
    Yevaluator.init(temp_global_stry);

    num_values = 0;
}

int visualizer::getYZeroOffset() {
     return h()/2;
}

int visualizer::getXZeroOffset() {
     return w()/2;
}

int visualizer::getNumValues() {
     return (Yevaluator.getNumValues() == Xevaluator.getNumValues() ? Yevaluator.getNumValues() : 0);
}

bool visualizer::UDFOccurred() {
     return Xevaluator.UDFOccurred() ||
            Yevaluator.UDFOccurred();
}

bool visualizer::FPEOccurred() {
     return Xevaluator.FPEOccurred() ||
            Yevaluator.FPEOccurred();
}

bool visualizer::parseErrorOccurred() {
     return Xevaluator.currentExpressionBad() ||
            Yevaluator.currentExpressionBad();
}

bool visualizer::evaluationErrorOccurred() {
     return FPEOccurred() || UDFOccurred();
}

void visualizer::resetInvalidIndices() {
     Xevaluator.resetInvalidIndices();
     Yevaluator.resetInvalidIndices();
}

void visualizer::draw() {
     num_values = getNumValues();

     fl_draw_box(FL_FLAT_BOX,x(),y(),w(),h(),FL_BLACK);

     fl_color(FL_WHITE);
     fl_font(FL_HELVETICA,16);

     int offsetx, offsety;
     int tooltip_width = 0;

     int64 yy,xx;

     if (!parseErrorOccurred()) {
         if (!evaluationErrorOccurred()) {

             fl_draw_box(FL_FLAT_BOX,x(),y() + getYZeroOffset(),w(),1,FL_DARK3);
             fl_draw_box(FL_FLAT_BOX,x() + getXZeroOffset(),y(),1,h(),FL_DARK3);

             for (int p = 0; p < num_values; p += dparams.incx) {
                  xx = Xevaluator.getValue(p);
                  yy = Yevaluator.getValue(p);
                  offsetx = (int)(dparams.vis_diffx*((ldouble)xx - dparams.minx)/(dparams.maxx - dparams.minx));
                  offsety = h() - (int)(dparams.vis_diffy*((ldouble)yy - dparams.miny)/(dparams.maxy - dparams.miny));

                  fl_draw_box(FL_FLAT_BOX,offsetx + box_locx,offsety + box_locy,1,1,FL_RED);
              }
         }
         else {
              if (FPEOccurred())
                  fl_draw("Floating Point Exception",x()+8,y()+24);
              else
                  fl_draw("Undefined Behavior Triggered (shift)",x()+8,y()+24);
         }
     }
     else {
          fl_draw("Parse Error",x()+8,y()+24);
     }

     if (show_tooltip) {
          tooltip_width = (int)cursor_str.size()*6;
          fl_draw_box(FL_EMBOSSED_BOX,getClocx()-tooltip_width,getClocy() - 12,tooltip_width,12,FL_WHITE);
          fl_color(FL_BLACK);
          fl_font(FL_HELVETICA,10);
          fl_draw(&cursor_str[0],getClocx()-tooltip_width+4,getClocy()-2);
     }
}

void visualizer::updateMinMaxValues() {
     auto maxy_value = Yevaluator.queryMaxValue();
     auto miny_value = Yevaluator.queryMinValue();
     auto maxx_value = Xevaluator.queryMaxValue();
     auto minx_value = Xevaluator.queryMinValue();

     dparams.maxy = (ldouble)(maxy_value <= MAXY_UPPER_BOUND ? maxy_value : MAXY_UPPER_BOUND);
     dparams.maxy = (dparams.maxy >= (ldouble)MAXY_LOWER_BOUND) ? dparams.maxy : (ldouble)MAXY_LOWER_BOUND;
     dparams.miny = (ldouble)(miny_value >= MINY_LOWER_BOUND ? miny_value : MINY_LOWER_BOUND);
     dparams.miny = (dparams.miny <= (ldouble)MINY_UPPER_BOUND) ? dparams.miny : (ldouble)MINY_UPPER_BOUND;
     if (std::abs(dparams.maxy) > std::abs(dparams.miny))
         dparams.miny = -1.0*dparams.maxy;
     else
         dparams.maxy = -1.0*dparams.miny;

     dparams.maxx = (ldouble)(maxx_value <= MAXX_UPPER_BOUND ? maxx_value : MAXX_UPPER_BOUND);
     dparams.maxx = (dparams.maxx >= (ldouble)MAXX_LOWER_BOUND) ? dparams.maxx : (ldouble)MAXX_LOWER_BOUND;
     dparams.minx = (ldouble)(minx_value >= MINX_LOWER_BOUND ? minx_value : MINX_LOWER_BOUND);
     dparams.minx = (dparams.minx <= (ldouble)MINX_UPPER_BOUND) ? dparams.minx : (ldouble)MINX_UPPER_BOUND;
     if (std::abs(dparams.maxx) > std::abs(dparams.minx))
         dparams.minx = -1.0*dparams.maxx;
     else
         dparams.maxx = -1.0*dparams.minx;
}

int visualizer::getClocx() {
     return Fl::event_x();
}

int visualizer::getClocy() {
     return Fl::event_y();
}

int visualizer::handle(int e) {
   switch (e) {
       case(FL_LEAVE): {
            show_tooltip = false;
            redraw();
            return 1;
       }
       case(FL_ENTER): {
            show_tooltip = true;
            redraw();
            return 1;
       }
       case(FL_MOVE): {
            std::string clocx_str = std::to_string((int64)((ldouble)((getClocx()-x())*(dparams.maxx - dparams.minx)/dparams.vis_diffx) + dparams.minx));
            std::string clocy_str = std::to_string((int64)((ldouble)((h() - getClocy() + y())*(dparams.maxy - dparams.miny)/dparams.vis_diffy) + dparams.miny));
            cursor_str = "(" + clocx_str + "," + clocy_str + ")";
            show_tooltip = true;
            redraw();
            return 1;
       }
       default:
            break;
   }
   return(Fl_Box::handle(e));
}

evaluator *visualizer::getXEvaluator() {
   return &Xevaluator;
}

evaluator *visualizer::getYEvaluator() {
   return &Yevaluator;
}

evaluator::evaluator() {
}

int64 evaluator::getValue(int i) {
   return values[i];
}

int64 evaluator::queryMinValue() {
   return *(std::min_element(values.begin(), values.end()));
}

int64 evaluator::queryMaxValue() {
   return *(std::max_element(values.begin(), values.end()));
}

void evaluator::init(std::string init_str) {
    fpe_index = udf_index = VALID_CONSTANT_IND;
    bad_expression = false;
    expression_str = init_str;
}

void evaluator::separateStringTokens(std::string str) {

    tokens.resize(0);
    parse_results.resize(0);
    expression_vec.resize(0);

    expression_str = str;

    bad_expression = checkInvalidChars() || (int)expression_str.size() == 0;

    if (bad_expression)
        return;

    expression_str += global_delim;

    auto curr_iter = expression_str.begin();
    auto base_iter = expression_str.begin();
    auto next_iter = expression_str.begin();

    for (;; curr_iter++) {
         next_iter++;

         auto curr_char = *curr_iter;
         auto base_char = *base_iter;
         auto next_char = *next_iter;

         auto base_digit = isHexDigit(base_char);
         auto next_digit = isHexDigit(next_char);

         bool curr_shift = ((curr_char == '>' || curr_char == '<') && (curr_char == next_char));

         if (!base_digit || !next_digit) {
             if (!curr_shift) {
                 expression_vec.push_back(std::string(base_iter,curr_iter+1));
             }
             else {
                 expression_vec.push_back(std::string(curr_iter,next_iter+1));
                 curr_iter++;
                 next_iter++;
             }
             base_iter = next_iter;
         }

         if (*next_iter == global_delim)
             break;
    }

    expression_vec.erase(std::remove(expression_vec.begin(), expression_vec.end(), " "), expression_vec.end());

    for (auto i = 0; i < (int)expression_vec.size(); ++i) {
         if (i > 0) {
             if (expression_vec[i] == "-" && expression_vec[i-1] == "-") {
                 bad_expression = true;
                 return;
             }
         }
    }

    for (auto i = 0; i < (int)expression_vec.size(); ++i) {
         if (expression_vec[i] == "-") {
             if (i == 0) {
                 expression_vec[i] = "u-";
             }
             else if (isPotentiallyBeforeUnaryMinus(expression_vec[i-1][0])) {
                 expression_vec[i] = "u-";
             }
         }
    }

    expression_str = "";

    for (const auto & x: expression_vec) {
         expression_str += x;

         token tok = {x,getTokenType(x)};

         if ((int)tok.value.size() > MAX_CONSTANT_HEXDIGITS || tok.value == "<" || tok.value == ">") {
             bad_expression = true;
             return;
         }

         tokens.push_back(tok);
    }

    expression_str.erase(std::remove(expression_str.begin(), expression_str.end(), 'u'), expression_str.end());

    populateParseTokens();

    bad_expression = checkInvalidExpression();
}

bool evaluator::checkInvalidChars() {
    for (const auto & c: expression_str) {
         if (!isHexDigit(c) && !isVariable(c) && !isOperatorChar(c) && !isParenthesis(c) && c != ' ') {
             return true;
         }
    }
    return false;
}

bool evaluator::currentExpressionBad() {
    return bad_expression;
}

bool evaluator::evaluationErrorOccurred() {
    return FPEOccurred() || UDFOccurred();
}

bool evaluator::FPEOccurred() {
    return fpe_index != VALID_CONSTANT_IND;
}

bool evaluator::UDFOccurred() {
    return udf_index != VALID_CONSTANT_IND;
}

void evaluator::populateParseTokens() {
    for (const auto & t: tokens) {
         switch(t.type) {
                case(TOKEN_CONSTANT_OPERAND):
                case(TOKEN_VARIABLE_OPERAND):
                     parse_results.push_back('x');
                     break;
                case(TOKEN_PARENTHESIS_OPEN):
                     parse_results.push_back('(');
                     break;
                case(TOKEN_PARENTHESIS_CLOSE):
                     parse_results.push_back(')');
                     break;
                case(TOKEN_INFIX_OPERATOR):
                     parse_results.push_back('i');
                     break;
                case(TOKEN_PREFIX_OPERATOR):
                     parse_results.push_back('p');
                     break;
                default:
                     break;
         }
    }
}

bool evaluator::checkInvalidExpression() {
    auto num_operands = std::count(parse_results.begin(), parse_results.end(), 'x');
    auto num_parenthso = std::count(parse_results.begin(), parse_results.end(), '(');
    auto num_parenthsc = std::count(parse_results.begin(), parse_results.end(), ')');
    auto num_infix_operators = std::count(parse_results.begin(), parse_results.end(), 'i');
    auto num_prefix_operators = std::count(parse_results.begin(), parse_results.end(), 'p');

    auto invalid_consecutive_tokens = false;

    if ((int)parse_results.size() > 1) {
         for (auto i = 0; i < (int)parse_results.size() - 1; ++i) {
              auto this_val = parse_results[i];
              auto next_val = parse_results[i+1];

              if ((this_val == 'x' && next_val == 'x') ||
                  (this_val == 'i' && next_val == 'i') ||
                  (this_val == '(' && next_val == ')') ||
                  (this_val == ')' && next_val == '(') ||
                  (this_val == 'x' && next_val == '(') ||
                  (this_val == ')' && next_val == 'x') ||
                  (this_val == 'i' && next_val == ')') ||
                  (this_val == '(' && next_val == 'i') ||
                  (this_val == 'p' && next_val == 'p') ||
                  (this_val == 'p' && next_val == 'i') ||
                  (this_val == 'p' && next_val == ')') ||
                  (this_val == 'x' && next_val == 'p') ||
                  (this_val == ')' && next_val == 'p')) {
                   invalid_consecutive_tokens = true;
                   break;
              }
         }
    }

    if (num_operands != num_infix_operators + 1)
        return true;

    if (num_parenthso != num_parenthsc)
        return true;

    if (invalid_consecutive_tokens)
        return true;

    return false;
}

void evaluator::simplifyExpression(int64 t) {
    operator_value current_operator;
    for (const auto & tok: tokens) {
         switch(tok.type) {
                case(TOKEN_CONSTANT_OPERAND):
                     operands.push(getint64FromHexStr(tok.value));
                     break;
                case(TOKEN_VARIABLE_OPERAND):
                     operands.push(t);
                     break;
                case(TOKEN_PARENTHESIS_OPEN):
                     operators.push(OPERATOR_OPP);
                     break;
                case(TOKEN_PARENTHESIS_CLOSE):
                     while(operators.top() != OPERATOR_OPP) {
                           performOperation(t);
                           if (evaluationErrorOccurred()) {
                               resetStacks();
                               return;
                           }
                     }
                     operators.pop();
                     break;
                default:
                     current_operator = getTokenOperatorEquivalent(tok);
                     while(canPerformScopeOperation(current_operator)) {
                           performOperation(t);
                           if (evaluationErrorOccurred()) {
                               resetStacks();
                               return;
                           }
                     }
                     operators.push(current_operator);
                     break;
         }
    }

    while((int)operators.size() > 0) {
          performOperation(t);
          if (evaluationErrorOccurred()) {
              resetStacks();
              return;
          }
    }

    values.push_back(operands.top());

    operands.pop();
}

void evaluator::performOperation(int64 t) {

    int64 result = 0LL;
    int64 v1,v2;
    operator_value op;

    v1 = operands.top();
    operands.pop();

    op = operators.top();
    if (op == OPERATOR_NEG) {
        v1 = -v1;
        operators.pop();
    }
    if (op == OPERATOR_NOT) {
        v1 = ~v1;
        operators.pop();
    }

    if ((int)operators.size() == 0) {
        operands.push(v1);
        return;
    }
    if ((int)operators.top() < 2) {
        operands.push(v1);
        return;
    }

    op = operators.top();
    operators.pop();

    v2 = operands.top();
    operands.pop();

    switch(op) {
           case(OPERATOR_MUL):
                result = v2 * v1;
                break;
           case(OPERATOR_DIV):
                if (v1 != 0LL)
                    result = v2 / v1;
                else {
                    fpe_index = t;
                    return;
                }
                break;
           case(OPERATOR_MOD):
                if (v1 != 0LL)
                    result = v2 % v1;
                else {
                    fpe_index = t;
                    return;
                }
                break;
           case(OPERATOR_ADD):
                result = v2 + v1;
                break;
           case(OPERATOR_SUB):
                result = v2 - v1;
                break;
           case(OPERATOR_LSF):
                if (v1 >= MIN_SHIFTABLE_BITS &&
                    v1 <= MAX_SHIFTABLE_BITS)
                    result = v2 << v1;
                else {
                    udf_index = t;
                    return;
                }
                break;
           case(OPERATOR_RSF):
                if (v1 >= MIN_SHIFTABLE_BITS &&
                    v1 <= MAX_SHIFTABLE_BITS)
                    result = v2 >> v1;
                else {
                    udf_index = t;
                    return;
                }
                break;
           case(OPERATOR_AND):
                result = v2 & v1;
                break;
           case(OPERATOR_XOR):
                result = v2 ^ v1;
                break;
           case(OPERATOR_IOR):
                result = v2 | v1;
                break;
           default:
                break;
    }

    operands.push(result);
}

void evaluator::resetStacks() {
    std::stack<operator_value>().swap(operators);
    std::stack<int64>().swap(operands);
}

void evaluator::clearValues() {
    values.resize(0);
}

void evaluator::resetInvalidIndices() {
    fpe_index = udf_index = VALID_CONSTANT_IND;
}

bool evaluator::canPerformScopeOperation(operator_value current_operator) {
     if ((int)operators.size() > 0) {
         if ((int)operators.top() >= 2) {
             return operator_prec[(int)operators.top()-2] <= operator_prec[(int)current_operator-2];
         }
     }
     return false;
}

int evaluator::getNumValues() {
     return (int)values.size();
}

std::string evaluator::getExpressionString() {
     return expression_str;
}

void evaluateParametricEquations(visualizer *vis) {
     vis->resetInvalidIndices();
     vis->getXEvaluator()->clearValues();
     vis->getYEvaluator()->clearValues();
     vis->getXEvaluator()->separateStringTokens(temp_global_strx);
     vis->getYEvaluator()->separateStringTokens(temp_global_stry);
     parse_success = !vis->parseErrorOccurred();
     if (parse_success) {
         for (int64 t = -2048LL; t <= 2048LL; ++t) {
              vis->getXEvaluator()->simplifyExpression(t);
              vis->getYEvaluator()->simplifyExpression(t);
              if (vis->evaluationErrorOccurred())
                  break;
         }
         temp_global_strx = vis->getXEvaluator()->getExpressionString();
         temp_global_stry = vis->getYEvaluator()->getExpressionString();
     }
     if (!vis->evaluationErrorOccurred() && parse_success)
         vis->updateMinMaxValues();
     vis->redraw();
}

void evaluateButton_CB(Fl_Widget *w, void *data) {
     Fl_Button * btn = (Fl_Button *)w;
     if (btn->value() == 1) {
         evaluateParametricEquations((visualizer *)data);
         if (parse_success) {
             inpx->value(&temp_global_strx[0]);
             inpy->value(&temp_global_stry[0]);
         }
     }
}

void modifyExpressionXString_CB(Fl_Widget *w, void *data) {
     Fl_Input * inp = (Fl_Input *)w;
     temp_global_strx = std::string(inp->value());
}

void modifyExpressionYString_CB(Fl_Widget *w, void *data) {
     Fl_Input * inp = (Fl_Input *)w;
     temp_global_stry = std::string(inp->value());
}

int main(int argc, char *argv[]) {
  Fl_Double_Window *window = new Fl_Double_Window(1024,600,"I64 parametric plotter");

  visualizer * vis = new visualizer(14,66,764,520);

  inpx = new Fl_Input(14,10,764,24);
  inpy = new Fl_Input(14,35,764,24);

  Fl_Button * btn = new Fl_Button(788,12,96,24,"EVALUATE");

  btn->when(FL_WHEN_CHANGED);
  btn->callback(evaluateButton_CB, vis);

  inpx->value(&temp_global_strx[0]);
  inpx->box(FL_UP_BOX);
  inpx->labelsize(12);

  inpx->when(FL_WHEN_ENTER_KEY_ALWAYS|inpx->when());
  inpx->callback(modifyExpressionXString_CB);

  inpy->value(&temp_global_stry[0]);
  inpy->box(FL_UP_BOX);
  inpy->labelsize(12);

  inpy->when(FL_WHEN_ENTER_KEY_ALWAYS|inpy->when());
  inpy->callback(modifyExpressionYString_CB);

  window->end();
  window->show();

  return Fl::run();
}

