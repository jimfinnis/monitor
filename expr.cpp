/**
 * @file
 * Expression parser.
 * 
 */

#include "expr.h"
#include "tokeniser.h"
#include "etokens.h"
#include "exception.h"

#include <QStack>
#include <QList>

/// the expression parser has its own tokeniser.

static Tokeniser etok;
static bool exprTokeniserInitialised=false;

/// error handler class for expression tokeniser

class ExprTError : public ITokeniserErrorHandler{
    virtual void HandleTokeniserError(class Tokeniser *t){
        throw Exception()
              .set("unexpected '%s' at line %d in config file",t->getstring(),t->getline());
    }
};

static ExprTError tokerrorhandler;


/// operators
enum Opcode {
    OP_ADD,
          OP_MUL,
          OP_DIV,
          OP_SUB,
          OP_LT,
          OP_GT,
          OP_LE,
          OP_GE,
          OP_NOT,
          OP_NEGATE,
          OP_EQ,
          OP_NE,
          OP_OR,
          OP_AND,
          OP_OPREN,
          OP_FLOATLIT,
          OP_GETFLOAT
};


/// instruction for expression VM
struct Instruction {
    /// operation
    Opcode op;
    /// data item
    union {
        float f;
        int i;
        DataBuffer<float> *fbuf;
    } d;
    
    Instruction(){
        d.i=-1;
    }
    
    Instruction(Opcode o){
        op = o;
    }
    
    Instruction(float n){
        op = OP_FLOATLIT;
        d.f = n;
    }
    Instruction(const Instruction &i){
        op=i.op;
        d.i = i.d.i;
    }
    Instruction(DataBuffer<float> *b){
        op = OP_GETFLOAT;
        d.fbuf = b;
    }
        
        
};

/// operators with priorities on the stack
struct StackOp {
    Opcode op;
    int pri;
    StackOp(Opcode o,int p){
        op=o;
        pri=p;
    }
    StackOp(){}
};

/// are we expecting a value? Used to differentiate unary and binary ops
bool expectingValue;

/// operator stack
static QStack<StackOp> opstack;

/// output list, built during construction.
static QList<Instruction *> *out;

/// shunting yard - handle an operator
static void handleBinop(Opcode op,int priority){
    if(expectingValue)
        throw Exception("invalid binary operator");
    
    while(!opstack.isEmpty() && priority>opstack.top().pri){
        out->append(new Instruction(opstack.pop().op));
    }
    opstack.push(StackOp(op,priority));
    expectingValue=true;
}

static const char *foo="+ * / - < > <=>=! ng= !=||&&OPLTFB";
void dumpst(){
    for(int i=0;i<opstack.size();i++){
        Opcode op = opstack[i].op;
        printf("%c%c ",foo[op*2],foo[op*2+1]);
    }
    printf("\n");
}

void dumpinst(int i,Instruction& inst){
    switch(inst.op){
    case OP_FLOATLIT:
        printf("%d opcode:%d  FLOAT %f\n",i,inst.op,inst.d.f);
        break;
    case OP_GETFLOAT:
        printf("%d opcode:%d  GETFLOAT %p\n",i,inst.op,inst.d.fbuf);
        break;
    default:
        printf("%d opcode:%d  %c%c\n",i,inst.op,foo[inst.op*2],foo[inst.op*2+1]);
    }
}


Expression::Expression(const char *s){
    expectingValue = true;
    str = strdup(s);
    
    // capacity of the buffer for this expression, gets incremented
    // by the cap of each buffer we depend on
    
    int capRequired = 0; // if it's a constant expression!
    
    out = new QList<Instruction *>();
    
    if(!exprTokeniserInitialised){
        exprTokeniserInitialised=true;
        
        etok.init();
        etok.seterrorhandler(&tokerrorhandler);
        etok.settokens(etokens);
        etok.setcommentlinesequence("#");
    }
    
    etok.reset(s);
    
    bool done = false;
    
    // this is the famous shunting yard algorithm.
    while(!done){
        etok.getnext();
//        printf("Token: %s  ",etok.getstring());
//        printf("Stack: ");dumpst();
        
        switch(etok.getcurrent()){
        case T_INT:
        case T_FLOAT:
            if(!expectingValue)
                throw Exception().set("unexpected '%s'",etok.getstring());
            out->append(new Instruction(etok.getfloat()));
            expectingValue=false;
            break;
        case T_IDENT:
            if(!expectingValue)
                throw Exception().set("unexpected '%s'",etok.getstring());
            expectingValue=false;
            {
                fflush(stdout);
                DataBuffer<float> *b = DataManager::findFloatBuffer(etok.getstring());
                if(!b)
                    throw Exception().set("cannot find variable '%s'",etok.getstring());
                b->addExpr(this);
                capRequired += b->capacity;
                out->append(new Instruction(b));
            }
            break;
        case T_MUL:
            handleBinop(OP_MUL,3);
            break;
        case T_ADD:
            handleBinop(OP_ADD,4);
            break;
        case T_AND:
            handleBinop(OP_AND,11);
            break;
        case T_OR:
            handleBinop(OP_OR,12);
            break;
        case T_SUB:
            if(expectingValue){
                // it's a unary munus
                opstack.push(StackOp(OP_NEGATE,2));
            } else
                handleBinop(OP_SUB,4);
            break;
        case T_DIV:
            handleBinop(OP_DIV,3);
            break;
            
        case T_EQ:
            handleBinop(OP_EQ,7);
            break;
        case T_LT:
            if(etok.getnext()==T_EQ)
                handleBinop(OP_LE,6);
            else{
                etok.rewind();
                handleBinop(OP_LT,6);
            }
            break;
        case T_GT:
            if(etok.getnext()==T_EQ)
                handleBinop(OP_GE,6);
            else{
                etok.rewind();
                handleBinop(OP_GT,6);
            }
            break;
        case T_PLING:
            if(etok.getnext()==T_EQ)
                handleBinop(OP_NE,7);
            else{
                etok.rewind();
                if(!expectingValue)
                    throw Exception("unexpected unary '!'");
                opstack.push(StackOp(OP_NOT,2));
            }
            break;
            
        case T_OPREN:
            if(!expectingValue)
                throw Exception("unexpected '('");
            opstack.push(StackOp(OP_OPREN,100));
            break;
        case T_CPREN:
            if(expectingValue)
                throw Exception("unexpected ')'");
            for(;;){
                if(opstack.isEmpty())
                    throw Exception().set("2mismatched parens");
                Opcode op = opstack.pop().op;
                if(op==OP_OPREN)break;
                out->append(new Instruction(op));
            }
            break;
        case T_END:
            done = true;
            break;
        }
    }
    while(!opstack.isEmpty()){
        Opcode op = opstack.pop().op;
        if(op==OP_OPREN)
            throw Exception().set("1mismatched parens");
        out->append(new Instruction(op));
    }
    
    instructions = (void *)out; 
    
    if(!capRequired)
        throw Exception("expression must have at least one variable");
    
    /// min and max set to zero, will be reset later.
    buffer = new DataBuffer<float>(RawDataBuffer::FLOAT,s,
                                   capRequired,0,0);
    
    
    
    printf("Disassembly for %s\n",str);
    for(int i=0;i<out->size();i++){
        Instruction *inst = (*out)[i];
        dumpinst(i,*inst);
    }
        
}

void Expression::recalc(){
    
    // stack of floats - bools are represented by floats where
    // +ve is true and -ve is false.
    
    QStack<float> st;
    
    double timeOfLatestChange=0;
    
    QList<Instruction *> *insts = (QList<Instruction *> *)instructions;
    for(int i=0;i<insts->size();i++){
        Instruction *ip = (*insts)[i];
        
        float a,b;
        
        
//        dumpinst(i,ip);
        
        switch(ip->op){
        case OP_ADD:
            st.push(st.pop() + st.pop());
            break;
        case OP_MUL:
            st.push(st.pop() * st.pop());
            break;
        case OP_DIV:
            b = st.pop();
            a = st.pop();
            st.push(a/b);
            break;
        case OP_SUB:
            b = st.pop();
            a = st.pop();
            st.push(a-b);
            break;
        case OP_LT:
            b = st.pop();
            a = st.pop();
            st.push(a<b?1:-1);
            break;
        case OP_GT:
            b = st.pop();
            a = st.pop();
            st.push(a>b?1:-1);
            break;
        case OP_LE:
            b = st.pop();
            a = st.pop();
            st.push(a<=b?1:-1);
            break;
        case OP_GE:
            b = st.pop();
            a = st.pop();
            st.push(a>=b?1:-1);
            break;
        case OP_EQ:
            b = st.pop();
            a = st.pop();
            st.push(a==b?1:-1);
            break;
        case OP_NE:
            b = st.pop();
            a = st.pop();
            st.push(a!=b?1:-1);
            break;
        case OP_AND:
            b = st.pop();
            a = st.pop();
            st.push((a>0 && b>0) ? 1 : -1);
            break;
        case OP_OR:
            b = st.pop();
            a = st.pop();
            st.push((a>0 || b>0) ? 1 : -1);
            break;
        case OP_NEGATE:
        case OP_NOT:
            st.push(-st.pop());
            break;
        case OP_FLOATLIT:
            st.push(ip->d.f);
            break;
        case OP_GETFLOAT:
            {
                Datum<float> *d = ip->d.fbuf->read(0);
                if(d){
                    if(d->t > timeOfLatestChange)
                        timeOfLatestChange=d->t;
                
                    st.push(d->d);
                }else
                      st.push(0); // undefined value is zero
            }
            break;
        case OP_OPREN:
            break;
        default:
            throw Exception().set("Bad opcode: %s",ip->op);
        }
    }
    
    float value = st.pop();
    
    Datum<float> *d = buffer->read(0);
    if(!d || d->t <= timeOfLatestChange)
        buffer->write(timeOfLatestChange,value);
    
}
