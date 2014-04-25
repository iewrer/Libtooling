//== CallGraph.cpp - AST-based Call graph  ----------------------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the AST-based CallGraph.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "CallGraph"

#include "CallGraph.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/StmtVisitor.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/GraphWriter.h"

using namespace clang;

STATISTIC(NumObjCCallEdges, "Number of Objective-C method call edges");
STATISTIC(NumBlockCallEdges, "Number of block call edges");

namespace {
/// A helper class, which walks the AST and locates all the call sites in the
/// given function body.
class CGBuilder : public StmtVisitor<CGBuilder> {
  CallGraph *G;
  CallGraphNode *CallerNode;

public:
  CGBuilder(CallGraph *g, CallGraphNode *N)
    : G(g), CallerNode(N) {}

  void VisitStmt(Stmt *S) { VisitChildren(S); }

  Decl *getDeclFromCall(CallExpr *CE) {
    if (FunctionDecl *CalleeDecl = CE->getDirectCallee())
      return CalleeDecl;

    // Simple detection of a call through a block.
    Expr *CEE = CE->getCallee()->IgnoreParenImpCasts();
    if (BlockExpr *Block = dyn_cast<BlockExpr>(CEE)) {
      NumBlockCallEdges++;
      return Block->getBlockDecl();
    }

    return 0;
  }

  void addCalledDecl(Decl *D) {
    if (G->includeInGraph(D)) {
      CallGraphNode *CalleeNode = G->getOrInsertNode(D);
      CallerNode->addCallee(CalleeNode, G);
    }
  }
//对于CallExpr来说，可能被调用的函数的定义在别处
//对于这样的函数调用，我们也应当把它包括进来
    void addCalledDeclForExpr(Decl *D) {
        CallGraphNode *CalleeNode = G->getOrInsertNode(D);
        CallerNode->addCallee(CalleeNode, G);
    }
  void VisitCallExpr(CallExpr *CE) {
    if (Decl *D = getDeclFromCall(CE))
      addCalledDeclForExpr(D);
  }

  // Adds may-call edges for the ObjC message sends.
  void VisitObjCMessageExpr(ObjCMessageExpr *ME) {
    if (ObjCInterfaceDecl *IDecl = ME->getReceiverInterface()) {
      Selector Sel = ME->getSelector();
      
      // Find the callee definition within the same translation unit.
      Decl *D = 0;
      if (ME->isInstanceMessage())
        D = IDecl->lookupPrivateMethod(Sel);
      else
        D = IDecl->lookupPrivateClassMethod(Sel);
      if (D) {
        addCalledDecl(D);
        NumObjCCallEdges++;
      }
    }
  }

  void VisitChildren(Stmt *S) {
    for (Stmt::child_range I = S->children(); I; ++I)
      if (*I)
        static_cast<CGBuilder*>(this)->Visit(*I);
  }
};

} // end anonymous namespace

void CallGraph::addNodesForBlocks(DeclContext *D) {
  if (BlockDecl *BD = dyn_cast<BlockDecl>(D))
    addNodeForDecl(BD, true);

  for (auto *I : D->decls())
    if (auto *DC = dyn_cast<DeclContext>(I))
      addNodesForBlocks(DC);
}

CallGraph::CallGraph() {
  Root = getOrInsertNode(0);
}

CallGraph::~CallGraph() {
  llvm::DeleteContainerSeconds(FunctionMap);
}
void CallGraph::mergeCallGraphNode(CallGraphNode* n1,CallGraphNode* n2) {
    FunctionDecl* callee = dyn_cast_or_null<FunctionDecl>(n1->getDecl());
    FunctionDecl* callee2 = dyn_cast_or_null<FunctionDecl>(n2->getDecl());
    if (!callee||!callee2) {
        return;
    }
    if (callee->getNameAsString()=="cyg_package_start") {
        llvm::errs() << callee2->getNameAsString();
        llvm::errs() << "hehe";
    }
    if (n1->empty()) {
        for (CallGraphNode::iterator it = n2->begin(); it != n2->end(); ++it) {
            n1->addCallee((*it), new CallGraph());
        }
    }
    else if(n2->empty()) {
        for (CallGraphNode::iterator it = n1->begin(); it != n1->end(); ++it) {
            n2->addCallee((*it), new CallGraph());
        }
    }
    if (n1->getParent().empty()&&!n2->getParent().empty()) {
        n1->setParent(n2->getParent());
        std::vector<CallGraphNode*> *tmp = &n2->getParent();
        for (std::vector<CallGraphNode*>::iterator it = tmp->begin(); it != tmp->end(); ++it) {
            CallGraphNode *parent = *it;
            parent->addCallee(n1, new CallGraph());
        }
    }
    if (n2->getParent().empty()&&!n1->getParent().empty()) {
        n2->setParent(n1->getParent());
        std::vector<CallGraphNode*> *tmp = &n1->getParent();
        for (std::vector<CallGraphNode*>::iterator it = tmp->begin(); it != tmp->end(); ++it) {
            CallGraphNode *parent = *it;
            parent->addCallee(n2, new CallGraph());
        }
    }
    if (!n1->getParent().empty()&&!n2->getParent().empty()) {
        std::vector<CallGraphNode*> *p1 = &n1->getParent();
        std::vector<CallGraphNode*> *p2 = &n2->getParent();
        //遍历n2的parent节点，分别将n1作为其子节点
        for (std::vector<CallGraphNode*>::iterator it = p2->begin(); it != p2->end(); ++it) {
            CallGraphNode *parent = *it;
            parent->addCallee(n1, new CallGraph());
        }
        //遍历n1的parent节点，分别将n2作为其子节点
        for (std::vector<CallGraphNode*>::iterator it = p1->begin(); it != p1->end(); ++it) {
            CallGraphNode *parent = *it;
            parent->addCallee(n2, new CallGraph());
        }
        //遍历n2的parent节点，添加到n1的parent列表中
        for (std::vector<CallGraphNode*>::iterator it = p2->begin(); it != p2->end(); ++it) {
            p1->push_back(*it);
        }
        //将这个parent并集也设置为n2的parent列表
        n2->setParent(*p1);
    }
}
void CallGraph::MergeFunctionMap() {
    llvm::errs() << "----------MergeFunctionMap test---------------\n";
    std::string name,name2;
    for (FunctionMapTy::iterator it = FunctionMap.begin(); it != FunctionMap.end(); ++it) {
        if(const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(it->first)) {
            name = ND->getNameAsString();
            llvm::errs() << name << "\n";
            if (name=="cyg_package_start") {
                if (const FunctionDecl *x = dyn_cast_or_null<FunctionDecl>(it->first)) {
                    if (x->isThisDeclarationADefinition()) {
                        llvm::errs() << name2;
                        llvm::errs() << "hehe";
                    }
                }
            }
        }
        for (FunctionMapTy::iterator it2 = FunctionMap.begin(); it2 != FunctionMap.end(); ++it2) {
            if(it2==it) {
                continue;
            }
            if(const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(it2->first)) {
                 name2 = ND->getNameAsString();
            }
            //如果两个函数都是类的成员函数，那么当函数名相同，且两个函数都是同一个类的成员时合并之
            //否则若函数名相同即可合并之
            if(name==name2) {
                if (name=="cyg_package_start") {
                    llvm::errs() << name2;
                    llvm::errs() << "hehe";
                }
                if (const CXXMethodDecl *n1 = dyn_cast_or_null<CXXMethodDecl>(it->first)) {
                    if(const CXXMethodDecl *n2 = dyn_cast_or_null<CXXMethodDecl>(it2->first)) {
                        std::string c1 = n1->getParent()->getNameAsString();
                        std::string c2 = n2->getParent()->getNameAsString();
                        if (c1 == c2) {
                            mergeCallGraphNode(it->second,it2->second);
                        }
                    }
                }
                else {
                    mergeCallGraphNode(it->second,it2->second);
                }
            }
        }
    }
    llvm::errs() << "----------MergeFunctionMap test finished---------------\n";
}
bool CallGraph::includeInGraph(const Decl *D) {
  assert(D);
    if (getNode(D)) {
        return false;
    }
  if (!D->getBody())
    return false;
//      return true;

  if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    // We skip function template definitions, as their semantics is
    // only determined when they are instantiated.
    if (!FD->isThisDeclarationADefinition() ||
        FD->isDependentContext())
      return false;

    IdentifierInfo *II = FD->getIdentifier();
    if (II && II->getName().startswith("__inline"))
      return false;
  }

  if (const ObjCMethodDecl *ID = dyn_cast<ObjCMethodDecl>(D)) {
    if (!ID->isThisDeclarationADefinition())
      return false;
  }

  return true;
}

void CallGraph::addNodeForDecl(Decl* D, bool IsGlobal) {
  assert(D);

  // Allocate a new node, mark it as root, and process it's calls.
  CallGraphNode *Node = getOrInsertNode(D);

  // Process all the calls by this function as well.
  CGBuilder builder(this, Node);
  if (Stmt *Body = D->getBody())
    builder.Visit(Body);
}

CallGraphNode *CallGraph::getNode(const Decl *F) const {
  FunctionMapTy::const_iterator I = FunctionMap.find(F);
  if (I == FunctionMap.end()) return 0;
  return I->second;
}

CallGraphNode *CallGraph::getOrInsertNode(Decl *F) {
  CallGraphNode *&Node = FunctionMap[F];
  if (Node)
    return Node;

  Node = new CallGraphNode(F);
  // Make Root node a parent of all functions to make sure all are reachable.
  if (F != 0)
    Root->addCallee(Node, this);
  return Node;
}

void CallGraph::print(raw_ostream &OS) const {
  OS << " --- Call graph Dump --- \n";

  // We are going to print the graph in reverse post order, partially, to make
  // sure the output is deterministic.
  llvm::ReversePostOrderTraversal<const clang::CallGraph*> RPOT(this);
  for (llvm::ReversePostOrderTraversal<const clang::CallGraph*>::rpo_iterator
         I = RPOT.begin(), E = RPOT.end(); I != E; ++I) {
    const CallGraphNode *N = *I;

    OS << "  Function: ";
    if (N == Root)
      OS << "< root >";
    else
      N->print(OS);

    OS << " calls: ";
    for (CallGraphNode::const_iterator CI = N->begin(),
                                       CE = N->end(); CI != CE; ++CI) {
      assert(*CI != Root && "No one can call the root node.");
      (*CI)->print(OS);
      OS << " ";
    }
    OS << '\n';
  }
  OS.flush();
}

void CallGraph::dump() const {
  print(llvm::errs());
}

void CallGraph::viewGraph() const {
  llvm::ViewGraph(this, "CallGraph");
}

void CallGraphNode::print(raw_ostream &os) const {
    if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(FD)){
//      return ND->printName(os);
        ND->printName(os);
        os << FD;
    };
    return;
  os << "< >";
}

void CallGraphNode::dump() const {
  print(llvm::errs());
}

namespace llvm {

template <>
struct DOTGraphTraits<const CallGraph*> : public DefaultDOTGraphTraits {

  DOTGraphTraits (bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getNodeLabel(const CallGraphNode *Node,
                                  const CallGraph *CG) {
    if (CG->getRoot() == Node) {
      return "< root >";
    }
    if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(Node->getDecl()))
      return ND->getNameAsString();
    else
      return "< >";
  }

};
}
