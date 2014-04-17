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
#include "CallGraph.h"
#include "LibtoolingClasses.h"
#include <string>

static cl::OptionCategory option("exmple");

int counts = 0;
vector<string> files;
vector<string>::iterator file_it;
vector<CXXRecordDecl*> recordDecls;

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
    traverse(cg.getRoot(), 0);
    
    return result;
}

