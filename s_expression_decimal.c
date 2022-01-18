
#include "mpc.h"
#include <editline/readline.h>

/* Add SYM and SEXPR as possible lval types */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR };

typedef union number {
    long lon;
    double dec;
}number;

typedef struct lval {
  int type;
  number num;
  short num_t;
  /* Error and Symbol types have some string data */
  char* err;
  char* sym;
  /* Count and Pointer to a list of "lval*"; */
  int count;
  struct lval** cell;
} lval;

/* Construct a pointer to a new Number lval */
lval* lval_num(double x){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    if (fmod(x, 1)>0) {
        v->num.dec = x;v->num_t = 2;
    }else { v->num.lon = x;v->num_t = 1; }
    return v;
}
/* All of these turns out are redundant
lval* lval_num_l(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
    v->num->l = x;v->num_t = 1;
  return v;
}
lval* lval_num_d(double x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
    v->num->d = x;v->num_t = 2;
  return v;
}*/

/* Arithmetic functions to support decimal point numbers properly */
void addition(lval* a,lval* b){
    if (a->num_t == 1 && b->num_t==1) {
        a->num.lon+=b->num.lon;
    }else if (a->num_t == 1 && b->num_t==2){
        a->num.dec=a->num.lon+b->num.dec;a->num_t=2;
    }else if (a->num_t == 2 && b->num_t==1){
        a->num.dec+=b->num.lon;
    }else{ a->num.dec+=b->num.dec; }
}

void subtraction(lval* a,lval* b){
    if (a->num_t == 1 && b->num_t==1) {
        a->num.lon-=b->num.lon;
    }else if (a->num_t == 1 && b->num_t==2){
        a->num.dec=a->num.lon-b->num.dec;a->num_t=2;
    }else if (a->num_t == 2 && b->num_t==1){
        a->num.dec-=b->num.lon;
    }else{ a->num.dec-=b->num.dec; }
}

void multiplication(lval* a,lval* b){
    if (a->num_t == 1 && b->num_t==1) {
        a->num.lon*=b->num.lon;
    }else if (a->num_t == 1 && b->num_t==2){
        a->num.dec=a->num.lon*b->num.dec;a->num_t=2;
    }else if (a->num_t == 2 && b->num_t==1){
        a->num.dec*=b->num.lon;
    }else{ a->num.dec*=b->num.dec; }
}

void division(lval* a,lval* b){
    if (a->num_t == 1 && b->num_t==1) {
        a->num.lon/=b->num.lon;
    }else if (a->num_t == 1 && b->num_t==2){
        a->num.dec=a->num.lon/b->num.dec;a->num_t=2;
    }else if (a->num_t == 2 && b->num_t==1){
        a->num.dec/=b->num.lon;
    }else{ a->num.dec/=b->num.dec; }
}

void remaining(lval* a,lval* b){
    if (a->num_t == 1 && b->num_t==1) {
        a->num.lon%=b->num.lon;
    }else if (a->num_t == 1 && b->num_t==2){
        a->num.dec=fmod(a->num.lon, b->num.dec);a->num_t=2;
    }else if (a->num_t == 2 && b->num_t==1){
        a->num.dec=fmod(a->num.dec, b->num.lon);
    }else{ a->num.dec=fmod(a->num.dec, b->num.dec); }
}

/* Construct a pointer to a new Error lval */
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

/* Construct a pointer to a new Symbol lval */
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

/* A pointer to a new empty Sexpr lval */
lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void lval_del(lval* v) {

  switch (v->type) {
    /* Do nothing special for number type */
    case LVAL_NUM: break;
    
    /* For Err or Sym free the string data */
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;
    
    /* If Sexpr then delete all elements inside */
    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      /* Also free the memory allocated to contain the pointers */
      free(v->cell);
    break;
  }
  
  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

lval* lval_add(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

lval* lval_pop(lval* v, int i) {
  /* Find the item at "i" */
  lval* x = v->cell[i];
  
  /* Shift memory after the item at "i" over the top */
  memmove(&v->cell[i], &v->cell[i+1],
    sizeof(lval*) * (v->count-i-1));
  
  /* Decrease the count of items in the list */
  v->count--;
  
  /* Reallocate the memory used */
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    
    /* Print Value contained within */
    lval_print(v->cell[i]);
    
    /* Don't print trailing space if last element */
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* v) {
  switch (v->type) {
    case LVAL_NUM:
          if (v->num_t == 1) {
              printf("%li", v->num.lon); break;
          } else { printf("%f", v->num.dec); break; }
    case LVAL_ERR:   printf("Error: %s", v->err); break;
    case LVAL_SYM:   printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
  }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

lval* builtin_op(lval* a, char* op) {
  
  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-number!");
    }
  }
  
  /* Pop the first element */
  lval* x = lval_pop(a, 0);
  
  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
      if (x->num_t == 1) {
          x->num.lon = -x->num.lon;
      } else { x->num.dec = -x->num.dec; }
  }
  
  /* While there are still elements remaining */
  while (a->count > 0) {
  
    /* Pop the next element */
    lval* y = lval_pop(a, 0);
    
    /* Perform operation */
    if (strcmp(op, "+") == 0) { addition(x,y); }
    if (strcmp(op, "-") == 0) { subtraction(x,y); }
    if (strcmp(op, "*") == 0) { multiplication(x,y); }
    if (strcmp(op, "%") == 0) { remaining(x,y); }
    if (strcmp(op, "/") == 0) {
        if (y->num.lon == 0 || y->num.dec == 0) {
        lval_del(x); lval_del(y);
        x = lval_err("Division By Zero.");
        break;
      }
      division(x,y);
    }
    
    /* Delete element now finished with */
    lval_del(y);
  }
  
  /* Delete input expression and return result */
  lval_del(a);
  return x;
}

lval* lval_eval(lval* v);

lval* lval_eval_sexpr(lval* v) {
  
  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(v->cell[i]);
  }
  
  /* Error Checking */
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }
  
  /* Empty Expression */
  if (v->count == 0) { return v; }
  
  /* Single Expression */
  if (v->count == 1) { return lval_take(v, 0); }
  
  /* Ensure First Element is Symbol */
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f); lval_del(v);
    return lval_err("S-expression Does not start with symbol.");
  }
  
  /* Call builtin with operator */
  lval* result = builtin_op(v, f->sym);
  lval_del(f);
  return result;
}

lval* lval_eval(lval* v) {
  /* Evaluate Sexpressions */
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
  /* All other lval types remain the same */
  return v;
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  double x = strtod(t->contents, NULL);
  return errno != ERANGE ?
    lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
  
  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
    
  /* My decimal part to read decimal as different kind of number*/
  //if (strstr(t->tag, "decimal")) { return lval_dec(t); } no need for now
  
  /* If root (>) or sexpr then create empty list */
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  
  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }
  
  return x;
}

int main(int argc, char** argv) {
  
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr  = mpc_new("sexpr");
  mpc_parser_t* Expr   = mpc_new("expr");
  mpc_parser_t* Lispy  = mpc_new("lispy");
  
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                          \
      number : /-?[0-9]+([.][0-9]+)?/ ;                    \
      symbol : '+' | '-' | '*' | '/' | '%' ;         \
      sexpr  : '(' <expr>* ')' ;               \
      expr   : <number> | <symbol> | <sexpr> ; \
      lispy  : /^/ <expr>* /$/ ;               \
    ",
    Number, Symbol, Sexpr, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.5");
  puts("Press Ctrl+c to Exit\n");
  
  while (1) {
  
    char* input = readline("lispy> ");
    add_history(input);
    
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      lval* x = lval_eval(lval_read(r.output));
      lval_println(x);
      lval_del(x);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
    
  }
  
  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
  
  return 0;
}
