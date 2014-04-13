//
//  main.cpp
//  Libtool
//
//  Created by Barry on 14-3-14.
//  Copyright (c) 2014年 Barry. All rights reserved.
//

//修改了CGBuilder::includeInGraph方法！
//它确定该节点是否应该加入到CG中

#include <iostream>
#include <dirent.h>
#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTImporter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
//#include "clang/Analysis/CallGraph.h"
#include "clang/Serialization/ASTWriter.h"
#include "clang/Serialization/ASTReader.h"
#include "clang/Frontend/ASTUnit.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/DenseMap.h"
#include "CallGraph.h"
#include <string>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using namespace clang::driver;
using namespace std;
using namespace serialization;

static cl::OptionCategory option("exmple");

int counts = 0;
vector<string> files;
vector<string>::iterator file_it;
vector<CXXRecordDecl*> recordDecls;
//insert_iterator<vector<CXXRecordDecl*>> record_it(recordDecls,recordDecls.begin());

class MyVisitor : public RecursiveASTVisitor<MyVisitor> {
    
public:
    virtual bool VisitVarDecl(VarDecl*node) {
        if (node->isFileVarDecl()) {
            QualType type = node->getType().getCanonicalType();
            if(type.getTypePtr()->isRecordType()&&node->isThisDeclarationADefinition()){
            	CXXRecordDecl* recordDecl = type->getAsCXXRecordDecl();
//                recordDecl->dumpColor();
                recordDecls.push_back(recordDecl);
//                record_it = recordDecl;
				string constructorClassName = type.getAsString();
                
				errs()<<"find a global variable type:"<< constructorClassName <<"\n";
				string constructorVarNames = node->getNameAsString();
				errs()<<"variable name:" << constructorVarNames << "\n";
            }
        }
        return true;
    }
    
};
class MyASTConsumer : public ASTConsumer {
    
//    MyVisitor *visitor;
    CompilerInstance& ci;
public:
    //因为该类中有内置成员（指向MyVisitor类型的指针）
    //因此必须要实现构造函数
    MyASTConsumer(CompilerInstance& c)
//    : visitor(new MyVisitor()), ci(c)// initialize the visitor
    : ci(c)
    { }
    ~MyASTConsumer() {
    }
    virtual void HandleTranslationUnit(ASTContext &Context){
        if (file_it!=files.end()) {
            StringRef file = StringRef(*file_it++);
            errs() << file.str()+"\n";
            //创建该文件对应的ASTUnit，并将其保存为AST File
            IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
            unique_ptr<ASTUnit> Unit(ASTUnit::LoadFromCompilerInvocation(&ci.getInvocation(), Diags));
            Unit->Save(file);
        }
//        files.push_back(file.str());
    }
};
class MyFrontendAction : public ASTFrontendAction {
 
public:
    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &C,StringRef file)
    {
        return new MyASTConsumer(C);
    }
};
void traverse(CallGraphNode* root,int count) {
    if (!root) {
        return;
    }
    int temp = count;
    for (CallGraphNode::iterator it = root->begin(); it != root->end(); ++it) {
        while (temp != 0) {
            errs() << "  ";
            temp--;
        }
        (*it)->print(errs());
        errs() << "\n";
        traverse((*it),count+1);
        temp = count;
    }
}
bool end_with(const std::string& str, const std::string& sub)
{
    size_t pos = str.rfind(sub);
    return pos == str.length() - sub.length();
}
void addAllCalledFunctions(CallGraphNode* root,CallGraph& cg,vector<Decl*>& used) {
    if (!root) {
        return;
    }
    for (CallGraphNode::iterator it = root->begin(); it != root->end(); ++it) {
        Decl* tmp = (*it)->getDecl() ;
        //如果被调用的函数是一个C++类的成员函数
        //则将该类的其他成员函数都加入到闭包中
        errs() << tmp << "\n";
        if (CXXMethodDecl* cxx = dyn_cast<CXXMethodDecl>(tmp)) {
            errs() << (cxx)->getNameAsString() << " " << (cxx) << "\n";
            CXXRecordDecl* parent = cxx->getParent();
            for (CXXRecordDecl::method_iterator methods = parent->method_begin(); methods != parent->method_end(); ++methods) {
                errs() << (*methods)->getNameAsString() << " " << (*methods) << "\n";
                if((*methods)!=cxx) {
                    CallGraphNode* Node = cg.getNode(*methods);
//                    errs() << Node->getDecl()->getAsFunction()->getNameAsString() << "\n";
                    if(find(used.begin(), used.end(), *methods)==used.end()) {
                        used.push_back(*methods);
                    }
                }
            }
        }
        //将被调用的函数加入到闭包中
        if (find(used.begin(), used.end(), tmp)==used.end()) {
            used.push_back(tmp);
        }
        //递归地将被调用函数的被调用函数加入到闭包中，直到再没有被调用函数为止
        addAllCalledFunctions((*it),cg,used);
    }
}
void findCloure(vector<Decl*>& start,CallGraph& cg) {
    for (vector<Decl*>::iterator it = start.begin(); it != start.end(); ++it) {
        //如果该函数的调用链尚未加入闭包，则加入之
        CallGraphNode *root = cg.getNode(*it);
        addAllCalledFunctions(root, cg, start);
    }
}
int main(int argc, const char * argv[])
{
    // insert code here...
    CommonOptionsParser op(argc,argv,option);

    CallGraph cg;
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
    vector<ASTUnit*> Units;
    vector<Decl*> unUsed;
    vector<string> src;

    
    ClangTool tool(op.getCompilations(),op.getSourcePathList());
    files = op.getSourcePathList();
    file_it = files.begin();

    for (vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
            (*it).append(".ast");
    }
//    newFrontendActionFactory<ASTMergeAction(MyFrontendAction, files)>();
    int result = tool.run(newFrontendActionFactory<MyFrontendAction>());
    for (vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
        //从AST File中读取并创建ASTUnit
        //采用OwingPrt智能指针保存创建出来Unit指针
        OwningPtr<ASTUnit> Unit(ASTUnit::LoadFromASTFile((*it), Diags, FileSystemOptions()));
        if (!Unit) {
            return 1;
        }
        //Unit.take()保证被取出的指针所指向的对象的所有权也被拿走
        //并将其保存到容器中
        Units.push_back(Unit.take());
    }
    
    //合并AST
    ASTUnit* all = *Units.begin();
    for (vector<ASTUnit*>::iterator it = Units.begin()+1; it!=Units.end(); ++it) {
        ASTImporter Importer(all->getASTContext(),all->getFileManager(),
                             (*it)->getASTContext(),(*it)->getFileManager(),false);
        TranslationUnitDecl *TU = all->getASTContext().getTranslationUnitDecl();
        for (auto *D : TU->decls()) {
            if (const auto *ND = dyn_cast<NamedDecl>(D))
                if (IdentifierInfo *II = ND->getIdentifier())
                    if (II->isStr("__va_list_tag") || II->isStr("__builtin_va_list"))
                        continue;
            Importer.Import(D);
        }
    }
    cg.addToCallGraph(all->getASTContext().getTranslationUnitDecl());
    cg.dump();
    MyVisitor *visitor = new MyVisitor();
    visitor->TraverseDecl(all->getASTContext().getTranslationUnitDecl());
        errs() << "------Traverse Of Classes finished--------\n";
    
    vector<Decl*> allFunctions;
    insert_iterator<vector<Decl*>> in(allFunctions,allFunctions.begin());
    
    for(vector<CXXRecordDecl*>::iterator it = recordDecls.begin(); it!= recordDecls.end(); ++it ){
    	for( CXXRecordDecl::method_iterator iter = (*it)->method_begin(); iter != (*it)->method_end(); ++iter){
            errs() << *iter << (*iter)->getNameAsString() << "\n";
            if (!(*it)->hasUserDeclaredCopyConstructor()) {
                if (CXXConstructorDecl* m = dyn_cast<CXXConstructorDecl>(*iter)) {
                    if(m->isCopyConstructor()) {
                        continue;
                    };
                }
            }
            in = *iter;
//            (*iter)->dumpColor();
//            errs() << (*iter)->getNameAsString() << "\n";
    	}
    }
    //找到最小闭包
    errs() << "------start to find closure of Used Functions--------\n";
    findCloure(allFunctions,cg);
    errs() << "------Call Graph Traverse Of Used Functions--------\n";
    errs() << allFunctions.size() <<"\n";
    //打印最小闭包中的函数调用关系
//    cg.dump();
    for (vector<Decl*>::iterator it = allFunctions.begin(); it != allFunctions.end(); ++it) {
        Decl* x = (*it);
//        x->dumpColor();
        CallGraphNode* root = cg.getNode((*it));
        if(root==0) {
            continue;
        }
//        if(!root->getDecl()) {
//            continue;
//        }
        errs() << root->getDecl()->getAsFunction()->getNameAsString() << "\n";
//        x->dumpColor();
//        traverse(root,0);
    }

    return result;
}

