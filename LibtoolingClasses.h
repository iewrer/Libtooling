//
//  LibtoolingClasses.h
//  FindBoundary
//
//  Created by Barry on 14-4-17.
//  Copyright (c) 2014年 Barry. All rights reserved.
//

#ifndef FindBoundary_LibtoolingClasses_h
#define FindBoundary_LibtoolingClasses_h

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
#include "clang/Serialization/ASTWriter.h"
#include "clang/Serialization/ASTReader.h"
#include "clang/Frontend/ASTUnit.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/DenseMap.h"
#include "CallGraph.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using namespace clang::driver;
using namespace std;
using namespace serialization;

#define PACKAGE_NUM 10
extern vector<string>::iterator file_it;
extern vector<string> files;

vector<FunctionDecl*> globalFuncs;
vector<VarDecl*> globalVars;

class MyVisitor : public RecursiveASTVisitor<MyVisitor> {
    
public:
    virtual bool VisitVarDecl(VarDecl*node) {
        if (node->isFileVarDecl() && ! node->isStaticDataMember()) {
        	for(vector<VarDecl*>::iterator it = globalVars.begin(); it != globalVars.end(); ++ it){
        		if((*it)->getType().getAsString() == node->getType().getAsString() && (*it)->getNameAsString() == node->getNameAsString()){
        			return true;
        		}
        	}
            globalVars.push_back(node);
        }
        return true;
    }
    
    virtual bool VisitFunctionDecl(FunctionDecl* node){
    	if(node->isDefined() && node->isGlobal()){
    		for(vector<FunctionDecl*>:: iterator it = globalFuncs.begin(); it != globalFuncs.end(); ++it){
    			if((*it)->getReturnType().getAsString() == node->getReturnType().getAsString() && (*it) ->getNameAsString() == node->getNameAsString()){
    				return true;
    			}
    		}

    		globalFuncs.push_back(node);
    	}
    	return true;
    }

};
class MyASTConsumer : public ASTConsumer {
    
    CompilerInstance& ci;
public:
    //因为该类中有内置成员（指向MyVisitor类型的指针）
    //因此必须要实现构造函数
    MyASTConsumer(CompilerInstance& c)
    : ci(c)// initialize the visitor
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
    }
};
class MyFrontendAction : public ASTFrontendAction {
    
public:
    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &C,StringRef file)
    {
        return new MyASTConsumer(C);
    }
};
#endif
