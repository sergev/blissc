//
//  expression.h
//  blissc
//
//  Created by Matthew Madison on 11/1/12.
//  Copyright (c) 2012 Matthew Madison. All rights reserved.
//

#ifndef blissc_expression_h
#define blissc_expression_h

#include "parser.h"
#include "storage.h"
#include "nametable.h"
#include "lexeme.h"

#undef DOEXPTYPE
#define DOEXPTYPES \
    DOEXPTYPE(NOOP) \
    DOEXPTYPE(PRIM_LIT) DOEXPTYPE(PRIM_SEG) \
    DOEXPTYPE(PRIM_RTNCALL) DOEXPTYPE(PRIM_BLK) \
    DOEXPTYPE(OPERATOR) \
    DOEXPTYPE(EXECFUN) DOEXPTYPE(CTRL_COND) \
    DOEXPTYPE(CTRL_LOOPWU) DOEXPTYPE(CTRL_LOOPID)
#define DOEXPTYPE(t_) EXPTYPE_##t_,
typedef enum {
    DOEXPTYPES
} exprtype_t;
#undef DOEXPTYPE
#define EXPTYPE_COUNT (EXPTYPE_CTRL_LOOPID+1)

#undef DOOPTYPE
#define DOOPTYPES \
    DOOPTYPE(FETCH) DOOPTYPE(UNARY_PLUS) DOOPTYPE(UNARY_MINUS) \
    DOOPTYPE(SHIFT) DOOPTYPE(MODULO) DOOPTYPE(MULT) DOOPTYPE(DIV) \
    DOOPTYPE(ADD) DOOPTYPE(SUBTRACT) \
    DOOPTYPE(CMP_EQL) DOOPTYPE(CMP_NEQ) \
    DOOPTYPE(CMP_LSS) DOOPTYPE(CMP_LEQ) \
    DOOPTYPE(CMP_GTR) DOOPTYPE(CMP_GEQ) \
    DOOPTYPE(CMP_EQLU) DOOPTYPE(CMP_NEQU) \
    DOOPTYPE(CMP_LSSU) DOOPTYPE(CMP_LEQU) \
    DOOPTYPE(CMP_GTRU) DOOPTYPE(CMP_GEQU) \
    DOOPTYPE(NOT) DOOPTYPE(AND) DOOPTYPE(OR) \
    DOOPTYPE(EQV) DOOPTYPE(XOR) \
    DOOPTYPE(ASSIGN)
#define DOOPTYPE(t_) OPER_##t_,
typedef enum {
    DOOPTYPES
} optype_t;
#undef DOOPTYPE
#define OPER_COUNT (OPER_ASSIGN+1)
#define OPER_NONE OPER_COUNT

struct expr_node_s;

struct expr_blk_s {
    struct expr_node_s *blkseq;
    block_t            *blkstg;
    scopectx_t          blkscope;
    strdesc_t          *codecomment;
};
struct expr_oper_s {
    struct expr_node_s *op_lhs, *op_rhs;
    optype_t            op_type;
};
struct expr_func_s {
    struct expr_node_s *func_arglist;
    // exfntype_t which;
};
struct expr_cond_s {
    struct expr_node_s  *test;
    struct expr_node_s  *cons;
    struct expr_node_s  *alt;
};
typedef enum {
    LOOP_PRETEST, LOOP_POSTTEST
} looptesttype_t;
struct expr_loopwu_s {
    struct expr_node_s  *test;
    struct expr_node_s  *body;
    looptesttype_t       type;
};
struct expr_loopid_s {
    scopectx_t          scope;
    struct expr_node_s  *init;
    struct expr_node_s  *term;
    struct expr_node_s  *step;
    struct expr_node_s  *body;
};
struct expr_rtncall_s {
    struct expr_node_s  *rtn;
    // need linkage XXX
    struct expr_node_s  *inargs;
    struct expr_node_s  *outargs;
    int                 incount, outcount;
};

struct expr_node_s {
    // for freelist tracking and sequences in blocks
    struct expr_node_s *next;
    exprtype_t          type;
    union {
        long            litval;
        seg_t           *segval;
        struct expr_blk_s  blkdata;
        struct expr_oper_s opdata;
        struct expr_func_s fdata;
        struct expr_cond_s cdata;
        struct expr_loopwu_s wudata;
        struct expr_loopid_s iddata;
        struct expr_rtncall_s rcdata;
    } data;
};
typedef struct expr_node_s expr_node_t;

const char *exprtype_name(exprtype_t type);
expr_node_t *expr_node_alloc(exprtype_t type);
void expr_node_free(expr_node_t *node, stgctx_t stg);


static inline __unused int expr_is_noop(expr_node_t *node) {
    return (node == 0 ? 1 : node->type == EXPTYPE_NOOP);
}
static inline __unused int expr_is_primary(expr_node_t *node) {
    return (node == 0 ? 0 : node->type >= EXPTYPE_PRIM_LIT && node->type <= EXPTYPE_PRIM_BLK);
}
static inline __unused int expr_is_opexp(expr_node_t *node) {
    return (node == 0 ? 0 : node->type == EXPTYPE_OPERATOR);
}
static inline __unused int expr_is_execfun(expr_node_t *node) {
    return (node == 0 ? 0 : node->type == EXPTYPE_EXECFUN);
}
static inline __unused int expr_is_ctrl(expr_node_t *node) {
    return (node == 0 ? 0 :
            (node->type >= EXPTYPE_CTRL_COND && node->type <= EXPTYPE_CTRL_LOOPID));
}

// Common fields
static inline __unused expr_node_t *expr_next(expr_node_t *node) {
    return node->next;
}
static inline __unused void expr_next_set(expr_node_t *node, expr_node_t *next) {
    node->next = next;
}
static inline __unused exprtype_t expr_type(expr_node_t *node) {
    return node->type;
}
static inline __unused void expr_type_set(expr_node_t *node, exprtype_t type) {
    node->type = type;
}
static inline __unused void *expr_data(expr_node_t *node) {
    return &node->data;
}
// PRIM_LIT
static inline __unused long expr_litval(expr_node_t *node) {
    return node->data.litval;
}
static inline __unused void expr_litval_set(expr_node_t *node, long value) {
    node->data.litval = value;
}
// PRIM_SEG
static inline __unused seg_t *expr_segval(expr_node_t *node) {
    return node->data.segval;
}
static inline __unused void expr_segval_set(expr_node_t *node, seg_t *segval) {
    node->data.segval = segval;
}
// PRIM_BLK
static inline __unused expr_node_t *expr_blk_seq(expr_node_t *node) {
    return node->data.blkdata.blkseq;
}
static inline __unused void expr_blk_seq_set(expr_node_t *node, expr_node_t *seq) {
    node->data.blkdata.blkseq = seq;
}
static inline __unused block_t *expr_blk_block(expr_node_t *node) {
    return node->data.blkdata.blkstg;
}
static inline __unused void expr_blk_block_set(expr_node_t *node, block_t *blk) {
    node->data.blkdata.blkstg = blk;
}
static inline __unused scopectx_t expr_blk_scope(expr_node_t *node) {
    return node->data.blkdata.blkscope;
}
static inline __unused void expr_blk_scope_set(expr_node_t *node, scopectx_t scope) {
    node->data.blkdata.blkscope = scope;
}
static inline __unused strdesc_t *expr_blk_codecomment(expr_node_t *node) {
    return node->data.blkdata.codecomment;
}
static inline __unused void expr_blk_codecomment_set(expr_node_t *node, strdesc_t *str) {
    node->data.blkdata.codecomment = str;
}

// PRIM_RTNCALL
static inline __unused expr_node_t *expr_rtnaddr(expr_node_t *node) {
    return node->data.rcdata.rtn;
}
static inline __unused void expr_rtnaddr_set(expr_node_t *node, expr_node_t *adr) {
    node->data.rcdata.rtn = adr;
}
static inline __unused int expr_inargs(expr_node_t *node, expr_node_t **arglst) {
    if (arglst != 0) *arglst = node->data.rcdata.inargs;
    return node->data.rcdata.incount;
}
static inline __unused void expr_inargs_set(expr_node_t *node, int count, expr_node_t *args) {
    node->data.rcdata.incount = count;
    node->data.rcdata.inargs = args;
}
static inline __unused int expr_outargs(expr_node_t *node, expr_node_t **arglst) {
    if (arglst != 0) *arglst = node->data.rcdata.outargs;
    return node->data.rcdata.outcount;
}
static inline __unused void expr_outargs_set(expr_node_t *node, int count, expr_node_t *args) {
    node->data.rcdata.outcount = count;
    node->data.rcdata.outargs = args;
}
// OPERATOR
static inline __unused expr_node_t *expr_op_lhs(expr_node_t *node) {
    return node->data.opdata.op_lhs;
}
static inline __unused void expr_op_lhs_set(expr_node_t *node, expr_node_t *child) {
    node->data.opdata.op_lhs = child;
}
static inline __unused expr_node_t *expr_op_rhs(expr_node_t *node) {
    return node->data.opdata.op_rhs;
}
static inline __unused void expr_op_rhs_set(expr_node_t *node, expr_node_t *child) {
    node->data.opdata.op_rhs = child;
}
static inline __unused optype_t expr_op_type(expr_node_t *node) {
    return (node->type == EXPTYPE_OPERATOR ? node->data.opdata.op_type : OPER_NONE);
}
static inline __unused void expr_op_type_set(expr_node_t *node, optype_t op) {
    node->data.opdata.op_type = op;
}
// CTRL_COND
static inline __unused expr_node_t *expr_cond_test(expr_node_t *node) {
    return node->data.cdata.test;
}
static inline __unused void expr_cond_test_set(expr_node_t *node, expr_node_t *test) {
    node->data.cdata.test = test;
}
static inline __unused expr_node_t *expr_cond_consequent(expr_node_t *node) {
    return node->data.cdata.cons;
}
static inline __unused void expr_cond_consequent_set(expr_node_t *node, expr_node_t *cons) {
    node->data.cdata.cons = cons;
}
static inline __unused expr_node_t *expr_cond_alternative(expr_node_t *node) {
    return node->data.cdata.alt;
}
static inline __unused void expr_cond_alternative_set(expr_node_t *node, expr_node_t *alt) {
    node->data.cdata.alt = alt;
}
// CTRL_WULOOP
static inline __unused expr_node_t *expr_wuloop_test(expr_node_t *node) {
    return node->data.wudata.test;
}
static inline __unused void expr_wuloop_test_set(expr_node_t *node, expr_node_t *test) {
    node->data.wudata.test = test;
}
static inline __unused expr_node_t *expr_wuloop_body(expr_node_t *node) {
   return node->data.wudata.body;
}
static inline __unused void expr_wuloop_body_set(expr_node_t *node, expr_node_t *body) {
    node->data.wudata.body = body;
}
static inline __unused looptesttype_t expr_wuloop_type(expr_node_t *node) {
    return node->data.wudata.type;
}
static inline __unused void expr_wuloop_type_set(expr_node_t *node,
                                                 looptesttype_t type) {
    node->data.wudata.type = type;
}
// CTRL_IDLOOP
static inline __unused scopectx_t expr_idloop_scope(expr_node_t *node) {
    return node->data.iddata.scope;
}
static inline __unused void expr_idloop_scope_set(expr_node_t *node, scopectx_t scope) {
    node->data.iddata.scope = scope;
}
static inline __unused expr_node_t *expr_idloop_init(expr_node_t *node) {
    return node->data.iddata.init;
}
static inline __unused void expr_idloop_init_set(expr_node_t *node, expr_node_t *init) {
    node->data.iddata.init = init;
}
static inline __unused expr_node_t *expr_idloop_term(expr_node_t *node) {
    return node->data.iddata.term;
}
static inline __unused void expr_idloop_term_set(expr_node_t *node, expr_node_t *term) {
    node->data.iddata.term = term;
}
static inline __unused expr_node_t *expr_idloop_step(expr_node_t *node) {
    return node->data.iddata.step;
}
static inline __unused void expr_idloop_step_set(expr_node_t *node, expr_node_t *step) {
    node->data.iddata.step = step;
}
static inline __unused expr_node_t *expr_idloop_body(expr_node_t *node) {
    return node->data.iddata.body;
}
static inline __unused void expr_idloop_body_set(expr_node_t *node, expr_node_t *body) {
    node->data.iddata.body = body;
}

void expr_init (scopectx_t kwdscope);
lextype_t expr_parse_next(parse_ctx_t pctx, lexeme_t **lexp, int longstrings_ok);
int expr_parse_block(parse_ctx_t pctx, expr_node_t **blkexp);
int parse_ctce(parse_ctx_t pctx, lexeme_t **lexp);
strdesc_t *expr_dumpinfo(expr_node_t *exp);

#endif
