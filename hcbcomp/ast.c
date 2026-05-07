#include "bcomp.h"
#include "ast.h"
#include <stdio.h>

// [English] Helper function to create a new AST node
// [Portuguese] Função auxiliar para criar um novo nó AST
static ast_node_t *ast_new(ast_op_t op)
{
    ast_node_t *node = (ast_node_t *)calloc(1, sizeof(ast_node_t));
    if (!node)
        error("out of memory");
    node->op = op;
    node->lvalue = 0;
    node->sym = NULL;
    node->int_value = 0;
    node->str_value = NULL;
    node->label = 0;
    node->left = NULL;
    node->right = NULL;
    node->cond = NULL;
    node->then = NULL;
    node->else_ = NULL;
    node->init = NULL;
    node->next = NULL;
    node->is_const = 0;
    node->const_value = 0;
    return node;
}

// [English] Creates an integer literal node
// [Portuguese] Cria um nó de literal inteiro
ast_node_t *ast_int(int value)
{
    ast_node_t *node = ast_new(AST_INT_LITERAL);
    node->int_value = value;
    node->is_const = 1;
    node->const_value = value;
    return node;
}

// [English] Creates a character literal node
// [Portuguese] Cria um nó de literal de caractere
ast_node_t *ast_char(int value)
{
    ast_node_t *node = ast_new(AST_CHAR_LITERAL);
    node->int_value = value;
    node->is_const = 1;
    node->const_value = value;
    return node;
}

// [English] Creates a string literal node
// [Portuguese] Cria um nó de literal de string
ast_node_t *ast_string(const char *value)
{
    ast_node_t *node = ast_new(AST_STRING_LITERAL);
    node->str_value = strdup(value);
    return node;
}

// [English] Creates an identifier node
// [Portuguese] Cria um nó de identificador
ast_node_t *ast_ident(symbol_t *sym)
{
    ast_node_t *node = ast_new(AST_IDENT);
    node->sym = sym;
    node->lvalue = 1;  // Identifiers are lvalues
    return node;
}

// [English] Creates a unary operator node
// [Portuguese] Cria um nó de operador unário
ast_node_t *ast_unary(ast_op_t op, ast_node_t *expr)
{
    ast_node_t *node = ast_new(op);
    node->left = expr;
    return node;
}

// [English] Creates a binary operator node
// [Portuguese] Cria um nó de operador binário
ast_node_t *ast_binary(ast_op_t op, ast_node_t *left, ast_node_t *right)
{
    ast_node_t *node = ast_new(op);
    node->left = left;
    node->right = right;
    return node;
}

// [English] Creates a function call node
// [Portuguese] Cria um nó de chamada de função
ast_node_t *ast_call(ast_node_t *func, ast_node_t *args)
{
    ast_node_t *node = ast_new(AST_CALL);
    node->left = func;
    node->right = args;  // args is a linked list
    return node;
}

// [English] Creates an array subscript node
// [Portuguese] Cria um nó de subscrito de array
ast_node_t *ast_index(ast_node_t *array, ast_node_t *index)
{
    ast_node_t *node = ast_new(AST_INDEX);
    node->left = array;
    node->right = index;
    node->lvalue = 1;  // Array subscript is an lvalue
    return node;
}

// [English] Creates a ternary operator node (a ? b : c)
// [Portuguese] Cria um nó de operador ternário (a ? b : c)
ast_node_t *ast_ternary(ast_node_t *cond, ast_node_t *then_, ast_node_t *else_)
{
    ast_node_t *node = ast_new(AST_COND);
    node->cond = cond;
    node->then = then_;
    node->else_ = else_;
    return node;
}

// [English] Creates an assignment node
// [Portuguese] Cria um nó de atribuição
ast_node_t *ast_assign(ast_node_t *left, ast_node_t *right)
{
    ast_node_t *node = ast_new(AST_ASSIGN);
    node->left = left;
    node->right = right;
    return node;
}

// [English] Creates an expression statement node
// [Portuguese] Cria um nó de comando de expressão
ast_node_t *ast_expr_stmt(ast_node_t *expr)
{
    ast_node_t *node = ast_new(AST_EXPR_STMT);
    node->left = expr;
    return node;
}

// [English] Creates an if statement node
// [Portuguese] Cria um nó de comando if
ast_node_t *ast_if(ast_node_t *cond, ast_node_t *then_, ast_node_t *else_)
{
    ast_node_t *node = ast_new(AST_IF);
    node->cond = cond;
    node->then = then_;
    node->else_ = else_;
    return node;
}

// [English] Creates a while statement node
// [Portuguese] Cria um nó de comando while
ast_node_t *ast_while(ast_node_t *cond, ast_node_t *body)
{
    ast_node_t *node = ast_new(AST_WHILE);
    node->cond = cond;
    node->then = body;  // reusing 'then' for body
    return node;
}

// [English] Creates a for statement node
// [Portuguese] Cria um nó de comando for
ast_node_t *ast_for(ast_node_t *init, ast_node_t *cond, ast_node_t *next, ast_node_t *body)
{
    ast_node_t *node = ast_new(AST_FOR);
    node->init = init;
    node->cond = cond;
    node->next = next;
    node->then = body;  // reusing 'then' for body
    return node;
}

// [English] Creates a return statement node
// [Portuguese] Cria um nó de comando return
ast_node_t *ast_return(ast_node_t *expr)
{
    ast_node_t *node = ast_new(AST_RETURN);
    node->left = expr;
    return node;
}

// [English] Creates a break statement node
// [Portuguese] Cria um nó de comando break
ast_node_t *ast_break(int label)
{
    ast_node_t *node = ast_new(AST_BREAK);
    node->label = label;
    return node;
}

// [English] Creates a block statement node
// [Portuguese] Cria um nó de bloco de comandos
ast_node_t *ast_block(ast_node_t *stmts)
{
    ast_node_t *node = ast_new(AST_BLOCK);
    node->left = stmts;  // stmts is a linked list
    return node;
}

// [English] Frees an AST node and its children
// [Portuguese] Libera um nó AST e seus filhos
void ast_free(ast_node_t *node)
{
    if (!node)
        return;

    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->cond);
    ast_free(node->then);
    ast_free(node->else_);
    ast_free(node->init);
    ast_free(node->next);

    if (node->str_value)
        free(node->str_value);

    free(node);
}

// [English] Deep copies an AST node
// [Portuguese] Copia profundamente um nó AST
ast_node_t *ast_copy(ast_node_t *node)
{
    if (!node)
        return NULL;

    ast_node_t *copy = ast_new(node->op);
    copy->lvalue = node->lvalue;
    copy->sym = node->sym;
    copy->int_value = node->int_value;
    if (node->str_value)
        copy->str_value = strdup(node->str_value);
    copy->label = node->label;
    copy->is_const = node->is_const;
    copy->const_value = node->const_value;

    copy->left = ast_copy(node->left);
    copy->right = ast_copy(node->right);
    copy->cond = ast_copy(node->cond);
    copy->then = ast_copy(node->then);
    copy->else_ = ast_copy(node->else_);
    copy->init = ast_copy(node->init);
    copy->next = ast_copy(node->next);

    return copy;
}

// [English] Dumps an AST for debugging (to stderr)
// [Portuguese] Imprime uma AST para depuração (para stderr)
static void ast_dump_indent(int indent)
{
    for (int i = 0; i < indent; i++)
        fprintf(stderr, "  ");
}

static const char *ast_op_name(ast_op_t op)
{
    switch (op)
    {
    case AST_INT_LITERAL: return "INT";
    case AST_CHAR_LITERAL: return "CHAR";
    case AST_STRING_LITERAL: return "STRING";
    case AST_IDENT: return "IDENT";
    case AST_NEG: return "NEG";
    case AST_NOT: return "NOT";
    case AST_LNOT: return "LNOT";
    case AST_PRE_INC: return "PRE_INC";
    case AST_PRE_DEC: return "PRE_DEC";
    case AST_POST_INC: return "POST_INC";
    case AST_POST_DEC: return "POST_DEC";
    case AST_DEREF: return "DEREF";
    case AST_ADDR: return "ADDR";
    case AST_ADD: return "ADD";
    case AST_SUB: return "SUB";
    case AST_MUL: return "MUL";
    case AST_DIV: return "DIV";
    case AST_MOD: return "MOD";
    case AST_AND: return "AND";
    case AST_OR: return "OR";
    case AST_XOR: return "XOR";
    case AST_SHL: return "SHL";
    case AST_SHR: return "SHR";
    case AST_EQ: return "EQ";
    case AST_NE: return "NE";
    case AST_LT: return "LT";
    case AST_GT: return "GT";
    case AST_LE: return "LE";
    case AST_GE: return "GE";
    case AST_ASSIGN: return "ASSIGN";
    case AST_ADD_ASSIGN: return "ADD_ASSIGN";
    case AST_SUB_ASSIGN: return "SUB_ASSIGN";
    case AST_MUL_ASSIGN: return "MUL_ASSIGN";
    case AST_DIV_ASSIGN: return "DIV_ASSIGN";
    case AST_MOD_ASSIGN: return "MOD_ASSIGN";
    case AST_AND_ASSIGN: return "AND_ASSIGN";
    case AST_OR_ASSIGN: return "OR_ASSIGN";
    case AST_XOR_ASSIGN: return "XOR_ASSIGN";
    case AST_CALL: return "CALL";
    case AST_INDEX: return "INDEX";
    case AST_LAND: return "LAND";
    case AST_LOR: return "LOR";
    case AST_COND: return "COND";
    case AST_EXPR_STMT: return "EXPR_STMT";
    case AST_IF: return "IF";
    case AST_WHILE: return "WHILE";
    case AST_FOR: return "FOR";
    case AST_RETURN: return "RETURN";
    case AST_BREAK: return "BREAK";
    case AST_BLOCK: return "BLOCK";
    default: return "UNKNOWN";
    }
}

void ast_dump(ast_node_t *node, int indent)
{
    if (!node)
    {
        ast_dump_indent(indent);
        fprintf(stderr, "NULL\n");
        return;
    }

    ast_dump_indent(indent);
    fprintf(stderr, "%s", ast_op_name(node->op));

    if (node->op == AST_INT_LITERAL || node->op == AST_CHAR_LITERAL)
        fprintf(stderr, "(%d)", node->int_value);
    else if (node->op == AST_STRING_LITERAL)
        fprintf(stderr, "(\"%s\")", node->str_value);
    else if (node->op == AST_IDENT && node->sym)
        fprintf(stderr, "(%s)", node->sym->name);
    else if (node->op == AST_BREAK)
        fprintf(stderr, "(L%d)", node->label);

    if (node->is_const)
        fprintf(stderr, " [const=%d]", node->const_value);

    fprintf(stderr, "\n");

    if (node->cond)
    {
        ast_dump_indent(indent + 1);
        fprintf(stderr, "cond:\n");
        ast_dump(node->cond, indent + 2);
    }
    if (node->left)
    {
        ast_dump_indent(indent + 1);
        fprintf(stderr, "left:\n");
        ast_dump(node->left, indent + 2);
    }
    if (node->right)
    {
        ast_dump_indent(indent + 1);
        fprintf(stderr, "right:\n");
        ast_dump(node->right, indent + 2);
    }
    if (node->then)
    {
        ast_dump_indent(indent + 1);
        fprintf(stderr, "then:\n");
        ast_dump(node->then, indent + 2);
    }
    if (node->else_)
    {
        ast_dump_indent(indent + 1);
        fprintf(stderr, "else:\n");
        ast_dump(node->else_, indent + 2);
    }
    if (node->init)
    {
        ast_dump_indent(indent + 1);
        fprintf(stderr, "init:\n");
        ast_dump(node->init, indent + 2);
    }
    if (node->next)
    {
        ast_dump_indent(indent + 1);
        fprintf(stderr, "next:\n");
        ast_dump(node->next, indent + 2);
    }
}

// [English] Evaluates an AST node at compile time if possible
// Returns 1 if successful, 0 if not evaluatable
// [Portuguese] Avalia um nó AST em tempo de compilação se possível
// Retorna 1 se bem-sucedido, 0 se não avaliável
int ast_eval(ast_node_t *node, int *result)
{
    if (!node)
        return 0;

    if (node->is_const)
    {
        *result = node->const_value;
        return 1;
    }

    switch (node->op)
    {
    case AST_INT_LITERAL:
    case AST_CHAR_LITERAL:
        *result = node->int_value;
        return 1;

    case AST_NEG:
        if (ast_eval(node->left, result))
        {
            *result = -*result;
            return 1;
        }
        break;

    case AST_NOT:
        if (ast_eval(node->left, result))
        {
            *result = ~*result;
            return 1;
        }
        break;

    case AST_LNOT:
        if (ast_eval(node->left, result))
        {
            *result = !*result;
            return 1;
        }
        break;

    case AST_ADD:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l + r;
                return 1;
            }
        }
        break;

    case AST_SUB:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l - r;
                return 1;
            }
        }
        break;

    case AST_MUL:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l * r;
                return 1;
            }
        }
        break;

    case AST_DIV:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r) && r != 0)
            {
                *result = l / r;
                return 1;
            }
        }
        break;

    case AST_MOD:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r) && r != 0)
            {
                *result = l % r;
                return 1;
            }
        }
        break;

    case AST_AND:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l & r;
                return 1;
            }
        }
        break;

    case AST_OR:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l | r;
                return 1;
            }
        }
        break;

    case AST_XOR:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l ^ r;
                return 1;
            }
        }
        break;

    case AST_SHL:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l << r;
                return 1;
            }
        }
        break;

    case AST_SHR:
        {
            int l, r;
            if (ast_eval(node->left, &l) && ast_eval(node->right, &r))
            {
                *result = l >> r;
                return 1;
            }
        }
        break;

    default:
        break;
    }

    return 0;
}

// [English] Optimizes an AST node (constant folding, algebraic simplification)
// Returns the optimized node (may be different from input)
// [Portuguese] Otimiza um nó AST (constant folding, simplificação algébrica)
// Retorna o nó otimizado (pode ser diferente da entrada)
ast_node_t *ast_optimize(ast_node_t *node)
{
    if (!node)
        return NULL;

    // First, recursively optimize children
    // Primeiro, otimiza recursivamente os filhos
    node->left = ast_optimize(node->left);
    node->right = ast_optimize(node->right);
    node->cond = ast_optimize(node->cond);
    node->then = ast_optimize(node->then);
    node->else_ = ast_optimize(node->else_);
    node->init = ast_optimize(node->init);
    node->next = ast_optimize(node->next);

    // Try to evaluate this node
    // Tenta avaliar este nó
    int result;
    if (ast_eval(node, &result))
    {
        node->is_const = 1;
        node->const_value = result;
        if (node->op == AST_INT_LITERAL || node->op == AST_CHAR_LITERAL)
            node->int_value = result;
    }

    // Algebraic simplifications
    // Simplificações algébricas
    switch (node->op)
    {
    case AST_ADD:
        // x + 0 → x
        if (node->right && node->right->is_const && node->right->const_value == 0)
        {
            ast_node_t *temp = node->left;
            node->left = NULL;
            ast_free(node->right);
            node->right = NULL;
            ast_free(node);
            return temp;
        }
        // 0 + x → x
        if (node->left && node->left->is_const && node->left->const_value == 0)
        {
            ast_node_t *temp = node->right;
            node->right = NULL;
            ast_free(node->left);
            node->left = NULL;
            ast_free(node);
            return temp;
        }
        break;

    case AST_SUB:
        // x - 0 → x
        if (node->right && node->right->is_const && node->right->const_value == 0)
        {
            ast_node_t *temp = node->left;
            node->left = NULL;
            ast_free(node->right);
            node->right = NULL;
            ast_free(node);
            return temp;
        }
        break;

    case AST_MUL:
        // x * 1 → x
        if (node->right && node->right->is_const && node->right->const_value == 1)
        {
            ast_node_t *temp = node->left;
            node->left = NULL;
            ast_free(node->right);
            node->right = NULL;
            ast_free(node);
            return temp;
        }
        // 1 * x → x
        if (node->left && node->left->is_const && node->left->const_value == 1)
        {
            ast_node_t *temp = node->right;
            node->right = NULL;
            ast_free(node->left);
            node->left = NULL;
            ast_free(node);
            return temp;
        }
        // x * 0 → 0
        if ((node->right && node->right->is_const && node->right->const_value == 0) ||
            (node->left && node->left->is_const && node->left->const_value == 0))
        {
            ast_free(node);
            return ast_int(0);
        }
        break;

    case AST_DIV:
        // x / 1 → x
        if (node->right && node->right->is_const && node->right->const_value == 1)
        {
            ast_node_t *temp = node->left;
            node->left = NULL;
            ast_free(node->right);
            node->right = NULL;
            ast_free(node);
            return temp;
        }
        break;

    case AST_SHL:
    case AST_SHR:
        // x << 0 → x, x >> 0 → x
        if (node->right && node->right->is_const && node->right->const_value == 0)
        {
            ast_node_t *temp = node->left;
            node->left = NULL;
            ast_free(node->right);
            node->right = NULL;
            ast_free(node);
            return temp;
        }
        break;

    default:
        break;
    }

    return node;
}

// [English] Forward declarations for code generation
// [Portuguese] Declarações antecipadas para geração de código
static void ast_gen_expr(ast_node_t *node);
static void ast_gen_stmt(ast_node_t *node);

// [English] Generates code for an expression AST node
// [Portuguese] Gera código para um nó de expressão AST
static void ast_gen_expr(ast_node_t *node)
{
    if (!node)
        return;

    // If node was optimized to a constant, just load it
    // Se o nó foi otimizado para uma constante, apenas carrega
    if (node->is_const && node->op != AST_IDENT)
    {
        gen_load_imm(node->const_value);
        return;
    }

    switch (node->op)
    {
    case AST_INT_LITERAL:
    case AST_CHAR_LITERAL:
        gen_load_imm(node->int_value);
        break;

    case AST_IDENT:
        if (node->sym)
        {
            if (node->sym->kind == SYM_LOCAL)
                gen_local_addr(node->sym->offset);
            else if (node->sym->kind == SYM_PARAM)
                gen_param_addr(node->sym->offset);
            else
                gen_load_addr(node->sym->name);
        }
        break;

    case AST_STRING_LITERAL:
        {
            int label = gen_label();
            gen_data();
            gen_label_int(label);
            gen_bytes(node->str_value);
            gen_word(0);
            gen_text();
            gen_load_label(label);
        }
        break;

    case AST_NEG:
        ast_gen_expr(node->left);
        gen_neg();
        break;

    case AST_NOT:
        ast_gen_expr(node->left);
        gen_not();
        break;

    case AST_LNOT:
        ast_gen_expr(node->left);
        gen_lnot();
        break;

    case AST_PRE_INC:
        ast_gen_expr(node->left);
        gen_push_prim();
        gen_deref();
        gen_push_prim();
        gen_load_imm(1);
        gen_pop_sec();
        gen_add();
        gen_pop_sec();
        gen_store_to_addr();
        gen_pop_sec();
        gen_exchange();
        break;

    case AST_PRE_DEC:
        ast_gen_expr(node->left);
        gen_push_prim();
        gen_deref();
        gen_push_prim();
        gen_load_imm(1);
        gen_pop_sec();
        gen_sub();
        gen_pop_sec();
        gen_store_to_addr();
        gen_pop_sec();
        gen_exchange();
        break;

    case AST_POST_INC:
        ast_gen_expr(node->left);
        gen_push_prim();
        gen_deref();
        gen_push_prim();
        gen_push_prim();
        gen_load_imm(1);
        gen_pop_sec();
        gen_add();
        gen_pop_sec();
        gen_store_to_addr();
        gen_pop_sec();
        gen_exchange();
        break;

    case AST_POST_DEC:
        ast_gen_expr(node->left);
        gen_push_prim();
        gen_deref();
        gen_push_prim();
        gen_push_prim();
        gen_load_imm(1);
        gen_pop_sec();
        gen_sub();
        gen_pop_sec();
        gen_store_to_addr();
        gen_pop_sec();
        gen_exchange();
        break;

    case AST_DEREF:
        ast_gen_expr(node->left);
        gen_deref();
        break;

    case AST_ADDR:
        // Already an address, just use it
        ast_gen_expr(node->left);
        break;

    case AST_ADD:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_add();
        break;

    case AST_SUB:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_sub();
        break;

    case AST_MUL:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_mul();
        break;

    case AST_DIV:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_div();
        break;

    case AST_MOD:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_mod();
        break;

    case AST_AND:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_and();
        break;

    case AST_OR:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_or();
        break;

    case AST_XOR:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_xor();
        break;

    case AST_SHL:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_shl();
        break;

    case AST_SHR:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_shr();
        break;

    case AST_EQ:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_cmp_eq();
        break;

    case AST_NE:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_cmp_ne();
        break;

    case AST_LT:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_cmp_lt();
        break;

    case AST_GT:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_cmp_gt();
        break;

    case AST_LE:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_cmp_le();
        break;

    case AST_GE:
        ast_gen_expr(node->left);
        gen_push_prim();
        ast_gen_expr(node->right);
        gen_pop_sec();
        gen_cmp_ge();
        break;

    case AST_ASSIGN:
        // Generate code for lvalue (address)
        ast_gen_expr(node->left);
        gen_push_prim();

        // Generate code for rvalue
        ast_gen_expr(node->right);

        // Store
        gen_pop_sec();
        gen_store_to_addr();
        break;

    case AST_ADD_ASSIGN:
    case AST_SUB_ASSIGN:
    case AST_MUL_ASSIGN:
    case AST_DIV_ASSIGN:
    case AST_MOD_ASSIGN:
    case AST_AND_ASSIGN:
    case AST_OR_ASSIGN:
    case AST_XOR_ASSIGN:
        // Load lvalue address
        ast_gen_expr(node->left);
        gen_push_prim();

        // Load current value
        gen_deref();
        gen_push_prim();

        // Load rvalue
        ast_gen_expr(node->right);
        gen_pop_sec();

        // Apply operation
        switch (node->op)
        {
        case AST_ADD_ASSIGN: gen_add(); break;
        case AST_SUB_ASSIGN: gen_sub(); break;
        case AST_MUL_ASSIGN: gen_mul(); break;
        case AST_DIV_ASSIGN: gen_div(); break;
        case AST_MOD_ASSIGN: gen_mod(); break;
        case AST_AND_ASSIGN: gen_and(); break;
        case AST_OR_ASSIGN: gen_or(); break;
        case AST_XOR_ASSIGN: gen_xor(); break;
        default: break;
        }

        // Store result
        gen_pop_sec();
        gen_store_to_addr();
        break;

    case AST_CALL:
        // Generate code for arguments (right-to-left)
        // Gerar código para argumentos (direita para esquerda)
        {
            int nargs = 0;
            ast_node_t *arg = node->right;
            while (arg)
            {
                nargs++;
                arg = arg->next;
            }

            // Generate arguments in reverse order
            // Gera argumentos em ordem inversa
            arg = node->right;
            ast_node_t **args = (ast_node_t **)malloc(nargs * sizeof(ast_node_t *));
            for (int i = 0; i < nargs; i++)
            {
                args[i] = arg;
                arg = arg->next;
            }

            for (int i = nargs - 1; i >= 0; i--)
            {
                ast_gen_expr(args[i]);
                gen_push_prim();
            }
            free(args);

            // Generate function name
            if (node->left && node->left->sym)
                gen_call(node->left->sym->name, nargs);
        }
        break;

    case AST_INDEX:
        // Array subscript: generate address of array, then add offset
        // Subscrito de array: gera endereço do array, depois adiciona offset
        ast_gen_expr(node->left);  // array address
        gen_push_prim();
        ast_gen_expr(node->right);  // index
        gen_push_prim();
        gen_load_imm(2);  // multiply by 2 for 16-bit elements
        gen_pop_sec();
        gen_mul();
        gen_pop_sec();
        gen_add();
        break;

    case AST_LAND:
        {
            int false_label = gen_label();
            int end_label = gen_label();

            // Left side
            ast_gen_expr(node->left);
            gen_push_prim();
            gen_load_imm(0);
            gen_pop_sec();
            gen_cmp_eq();
            gen_jz(false_label);

            // Right side
            ast_gen_expr(node->right);
            gen_jmp(end_label);

            // False branch
            gen_label_int(false_label);
            gen_load_imm(0);

            // End
            gen_label_int(end_label);
        }
        break;

    case AST_LOR:
        {
            int true_label = gen_label();
            int end_label = gen_label();

            // Left side
            ast_gen_expr(node->left);
            gen_push_prim();
            gen_load_imm(0);
            gen_pop_sec();
            gen_cmp_ne();
            gen_jz(true_label);

            // Right side
            ast_gen_expr(node->right);
            gen_jmp(end_label);

            // True branch
            gen_label_int(true_label);
            gen_load_imm(1);

            // End
            gen_label_int(end_label);
        }
        break;

    case AST_COND:
        {
            int false_label = gen_label();
            int end_label = gen_label();

            // Condition
            ast_gen_expr(node->cond);
            gen_push_prim();
            gen_load_imm(0);
            gen_pop_sec();
            gen_cmp_eq();
            gen_jz(false_label);

            // False branch
            ast_gen_expr(node->else_);
            gen_jmp(end_label);

            // True branch
            gen_label_int(false_label);
            ast_gen_expr(node->then);

            // End
            gen_label_int(end_label);
        }
        break;

    default:
        break;
    }
}

// [English] Generates code for a statement AST node
// [Portuguese] Gera código para um nó de comando AST
static void ast_gen_stmt(ast_node_t *node)
{
    if (!node)
        return;

    switch (node->op)
    {
    case AST_EXPR_STMT:
        ast_gen_expr(node->left);
        break;

    case AST_IF:
        {
            int else_label = gen_label();
            int end_label = gen_label();

            // Condition
            ast_gen_expr(node->cond);
            gen_push_prim();
            gen_load_imm(0);
            gen_pop_sec();
            gen_cmp_eq();
            gen_jz(else_label);

            // Then branch
            ast_gen_stmt(node->then);
            gen_jmp(end_label);

            // Else branch
            gen_label_int(else_label);
            if (node->else_)
                ast_gen_stmt(node->else_);

            // End
            gen_label_int(end_label);
        }
        break;

    case AST_WHILE:
        {
            int start_label = gen_label();
            int end_label = gen_label();

            // Start
            gen_label_int(start_label);

            // Condition
            ast_gen_expr(node->cond);
            gen_push_prim();
            gen_load_imm(0);
            gen_pop_sec();
            gen_cmp_eq();
            gen_jz(end_label);

            // Body
            ast_gen_stmt(node->then);

            // Loop back
            gen_jmp(start_label);

            // End
            gen_label_int(end_label);
        }
        break;

    case AST_FOR:
        {
            int start_label = gen_label();
            int cond_label = gen_label();
            int end_label = gen_label();

            // Initialization
            if (node->init)
                ast_gen_stmt(node->init);

            // Condition check
            gen_label_int(cond_label);
            if (node->cond)
            {
                ast_gen_expr(node->cond);
                gen_push_prim();
                gen_load_imm(0);
                gen_pop_sec();
                gen_cmp_eq();
                gen_jz(end_label);
            }

            // Body
            ast_gen_stmt(node->then);

            // Next expression
            gen_label_int(start_label);
            if (node->next)
                ast_gen_expr(node->next);

            // Loop back to condition
            gen_jmp(cond_label);

            // End
            gen_label_int(end_label);
        }
        break;

    case AST_RETURN:
        if (node->left)
            ast_gen_expr(node->left);
        gen_return();
        break;

    case AST_BREAK:
        gen_jmp(node->label);
        break;

    case AST_BLOCK:
        {
            ast_node_t *stmt = node->left;
            while (stmt)
            {
                ast_gen_stmt(stmt);
                stmt = stmt->next;
            }
        }
        break;

    default:
        break;
    }
}

// [English] Main entry point for AST code generation
// [Portuguese] Ponto de entrada principal para geração de código AST
void ast_gen(ast_node_t *node)
{
    if (!node)
        return;

    // Optimize the AST first
    // Otimiza a AST primeiro
    ast_node_t *optimized = ast_optimize(node);

    // Generate code
    // Gera código
    if (optimized->op == AST_BLOCK || optimized->op == AST_IF ||
        optimized->op == AST_WHILE || optimized->op == AST_FOR ||
        optimized->op == AST_RETURN || optimized->op == AST_BREAK ||
        optimized->op == AST_EXPR_STMT)
    {
        ast_gen_stmt(optimized);
    }
    else
    {
        ast_gen_expr(optimized);
    }
}
