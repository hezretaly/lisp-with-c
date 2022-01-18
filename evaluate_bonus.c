#include "mpc.h"

#include <editline/readline.h>

int number_of_nodes(mpc_ast_t* t) {
  if (t->children_num == 0) { return 1; }
  if (t->children_num >= 1) {
    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}

int number_of_leaves(mpc_ast_t* t){
    if (t->children_num == 0) {
        if(strstr(t->tag, "number")) {
            printf("ey");
            return 1;
        }
        else {
        return 0; }
    }
    
    
    if (t->children_num >= 1) {
    int total = 0;
    for (int i = 0; i < t->children_num; i++) {
        total = total + number_of_leaves(t->children[i]);
    }
    return total;
    }
    return 0;
}

int number_of_branches(mpc_ast_t* t) {
    if(t->children_num == 0) return 0;
  if (t->children_num >= 1) {
    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_branches(t->children[i]);
    }
    return total;
  }
  return 0;
}

void number_of_biggest(mpc_ast_t* t, int *b) {
  if (t->children_num == 0)  return;
  if (t->children_num >= 1) {
    if(t->children_num > *b) *b=t->children_num;
    for (int i = 0; i < t->children_num; i++) {
       number_of_biggest(t->children[i],b);
    }
  }
}

/* Use operator string to see which operation to perform */
long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  return 0;
}

long eval(mpc_ast_t* t) {
  
  /* If tagged as number return it directly. */ 
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }
  
  /* The operator is always second child. */
  char* op = t->children[1]->contents;
  
  /* We store the third child in `x` */
  long x = eval(t->children[2]);
  
  /* Iterate the remaining children and combining. */
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
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.3");
  puts("Press Ctrl+c to Exit\n");
  
  while (1) {
  
    char* input = readline("lispy> ");
    add_history(input);
    
    mpc_result_t r;
    
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      
      //long result = eval(r.output);
     // printf("%li\n", result); 
        /* Load AST from output */
        mpc_ast_t* a = r.output;
        printf("Tag: %s\n", a->tag);
        printf("Contents: %s\n", a->contents);
        printf("Number of children: %i\n", a->children_num);

        /* Get First Child */
       int biggests=0;
       number_of_biggest(a,&biggests);
       printf("biggest spanning subtree: %d\n", biggests);
        printf("nob: %d\n", number_of_branches(a));
        printf("nol: %d\n", number_of_leaves(a));
        printf("non: %d\n", number_of_nodes(a));
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