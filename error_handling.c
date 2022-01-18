#include "mpc.h"
#include <editline/readline.h>

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR, LVAL_DEC };

/* Declare New lval Struct */
typedef struct {
    int type;
    long num;
    double dec;
    int err;
} lval;

/* helper function find if integer or not*/
int isLong(double val)
{
    long truncated = (long)val;
    return (val == truncated)? 1 : 0;
}

/* Create a new number type lval */
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Create a new decimal type lval */
lval lval_dec(double x){
    lval v;
    v.type = LVAL_DEC;
    v.dec = x;
    return v;
}

/* Operations functions to ease the condition cases */
lval multiplication(lval x, lval y){
    if (x.type == LVAL_DEC && y.type == LVAL_DEC){
        return lval_dec(x.dec * y.dec);
    } else if(x.type == LVAL_DEC){
        return lval_dec(x.dec * y.num);
    } else if(y.type == LVAL_DEC){
        return lval_dec(x.num * y.dec);
    } else {return lval_num(x.num * y.num);}
}

lval addition(lval x, lval y){
    if (x.type == LVAL_DEC && y.type == LVAL_DEC){
        return lval_dec(x.dec + y.dec);
    } else if(x.type == LVAL_DEC){
        return lval_dec(x.dec + y.num);
    } else if(y.type == LVAL_DEC){
        return lval_dec(x.num + y.dec);
    } else {return lval_num(x.num + y.num);}
}

lval subtraction(lval x, lval y){
    if (x.type == LVAL_DEC && y.type == LVAL_DEC){
        return lval_dec(x.dec - y.dec);
    } else if(x.type == LVAL_DEC){
        return lval_dec(x.dec - y.num);
    } else if(y.type == LVAL_DEC){
        return lval_dec(x.num - y.dec);
    } else {return lval_num(x.num - y.num);}
}

lval division(lval x, lval y){
    if (x.type == LVAL_DEC && y.type == LVAL_DEC){
        return lval_dec(x.dec / y.dec);
    } else if(x.type == LVAL_DEC){
        return lval_dec(x.dec / y.num);
    } else if(y.type == LVAL_DEC){
        return lval_dec(x.num / y.dec);
    } else {return lval_dec(x.num / y.num);}
}

/* Print an "lval" */
void lval_print(lval v) {
  switch (v.type) {
    /* In the case the type is a number print it */
    /* Then 'break' out of the switch. */
    case LVAL_NUM: printf("%li", v.num); break;
    
    /* In case it is decimal */
    case LVAL_DEC: printf("%f", v.dec); break;
    /* In the case the type is an error */
    case LVAL_ERR:
      /* Check what type of error it is and print it */
      if (v.err == LERR_DIV_ZERO) {
        printf("Error: Division By Zero!");
      }
      if (v.err == LERR_BAD_OP)   {
        printf("Error: Invalid Operator!");
      }
      if (v.err == LERR_BAD_NUM)  {
        printf("Error: Invalid Number!");
      }
    break;
  }
}

/* Print an "lval" followed by a newline */
void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval_op(lval x, char* op, lval y) {
  
  /* If either value is an error return it */
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }
  if (x.type == LVAL_NUM && y.type == LVAL_NUM) {
        /* Otherwise do maths on the number values */
        if (strcmp(op, "+") == 0) { return addition(x, y); }
        if (strcmp(op, "-") == 0) { return subtraction(x, y); }
        if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
        if (strcmp(op, "*") == 0) { return multiplication(x , y); }
        if (strcmp(op, "^") == 0) { return lval_num(pow(x.num,y.num)); }
        if (strcmp(op, "max") == 0) { return lval_num(x.num>y.num?x.num:y.num); }
        if (strcmp(op, "min") == 0) { return lval_num(x.num<y.num?x.num:y.num); }
        if (strcmp(op, "/") == 0) {
          /* If second operand is zero return error */
          return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : division(x, y);
        }
  } else if (x.type == LVAL_DEC && y.type == LVAL_NUM){
      /* Otherwise do maths on the number values */
         if (strcmp(op, "+") == 0) { return lval_dec(x.dec + y.num); }
         if (strcmp(op, "-") == 0) { return lval_dec(x.dec - y.num); }
         if (strcmp(op, "%") == 0) { return lval_num((long) x.dec %  y.num); }
         if (strcmp(op, "*") == 0) { return multiplication(x , y); }
         if (strcmp(op, "^") == 0) { return lval_dec(pow(x.dec,y.num)); }
         if (strcmp(op, "max") == 0) { return lval_num(x.dec>y.num?x.dec:y.num); }
         if (strcmp(op, "min") == 0) { return lval_num(x.dec<y.num?x.dec:y.num); }
         if (strcmp(op, "/") == 0) {
           /* If second operand is zero return error */
           return y.num == 0
             ? lval_err(LERR_DIV_ZERO)
             : lval_num(x.dec / y.num);
         }
  }
  
  
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
  
  if (strstr(t->tag, "number")) {
    /* Check if there is some error in conversion */
    errno = 0;
      if (!isLong(strtod(t->contents, NULL))) {
          double x = strtod(t->contents, NULL);
          return errno != ERANGE ? lval_dec(x) : lval_err(LERR_BAD_NUM);
      }
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }
  
  char* op = t->children[1]->contents;  
  lval x = eval(t->children[2]);
  
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  
  return x;  
}

int main(int argc, char** argv) {
  
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");
  
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+([.][0-9]+)?/ ;                             \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | \"max\" | \"min\" ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.4");
  puts("Press Ctrl+c to Exit\n");
  
  while (1) {
  
    char* input = readline("lispy> ");
    add_history(input);
    
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
      
    } else {    
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
    
  }
  
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  
  return 0;
}
