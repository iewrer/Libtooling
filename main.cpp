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
#include <fstream>
#include "CallGraph.h"
#include "LibtoolingClasses.h"
#include <string>
#include <map>

static cl::OptionCategory option("exmple");


vector<string> files;
vector<string>::iterator file_it;

map<Decl*,string> funcPackageMap;

string keys[] = {"devs", "error", "fs", "hal", "infra", "io", "isoinfra", "kernel", "language", "services"};

map<FunctionDecl*, FunctionDecl*> *callRelation[PACKAGE_NUM][PACKAGE_NUM];
vector<FunctionDecl*> * rootFunctions[PACKAGE_NUM];

int getTypeIndex(string type);
void traverseFunction(CallGraphNode* root);
int main(int argc, const char * argv[]);
void findMargin(CallGraphNode * root);
void findCallee(CallGraphNode *root, vector<FunctionDecl*> & callees);
void findRootCaller(vector<FunctionDecl*> & callees);
void printMargin();
void printRootCaller();
void printMarginDot();

void traverse(CallGraphNode* root,int count) {
    if (!root) {
        return;
    }
    int temp = count;
    for (CallGraphNode::iterator it = root->begin(); it != root->end(); ++it) {
    	FunctionDecl* rootDecl = dyn_cast_or_null<FunctionDecl>(root->getDecl());
    	FunctionDecl* itDecl = dyn_cast_or_null<FunctionDecl>((*it)->getDecl());
        
    	if(rootDecl && itDecl && rootDecl->getNameAsString() == itDecl->getNameAsString())
    		continue;
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



//遍历CallGraph
void traverseFunction(CallGraphNode* root,int count) {
    if (!root) {
        return;
    }
    int temp = count;
    for (CallGraphNode::iterator it = root->begin(); it != root->end(); ++it) {
        
    	FunctionDecl* rootDecl = dyn_cast_or_null<FunctionDecl>(root->getDecl());
    	FunctionDecl* tmp = dyn_cast_or_null<FunctionDecl>((*it)->getDecl());
    	if(rootDecl && tmp && rootDecl->getNameAsString() == tmp->getNameAsString())
    		continue;
//        while (temp != 0) {
//            errs() << "  ";
//            temp--;
//        }
//        if (rootDecl) {
//            errs() << count << "root: " << rootDecl->getNameAsString() << " ";
//        }
//        if (tmp) {
//            errs() << "tmp: " << tmp->getNameAsString() << "\n";
//        }
    	else {
            //如果该函数声明是一个定义
            //则找到它对应的包名称
            //将其加入map:funcPackageMap中
            if (tmp->isThisDeclarationADefinition()){
                
                string package = "empty";
                string filename = (*it)->getDecl()->getASTContext().getSourceManager().getPresumedLoc((*it)->getDecl()->getSourceRange().getBegin()).getFilename();
                for (int i = 0; i < PACKAGE_NUM; i++) {
                    if (filename.find(keys[i])!= filename.npos) {
                        package = keys[i];
                        break;
                    }
                }
                if (filename.find("mathincl")!= filename.npos)
                	package = "language";
                if (filename.find("libc")!= filename.npos)
                	package = "language";
                if (filename.find("memalloc")!= filename.npos)
                    package = "services";
                if (filename.find("math.h") != filename.npos)
                	package = "language";
                
                errs() << tmp->getNameAsString() << " ";
                errs() << package << "\n";
                funcPackageMap.insert(make_pair((*it)->getDecl(), package));
            }
            
        }
        temp = count;
        traverseFunction((*it),count+1);
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
    for (vector<ASTUnit*>::iterator it = Units.begin(); it != Units.end(); ++it) {
        cg.addToCallGraph((*it)->getASTContext().getTranslationUnitDecl());
    }

    cg.MergeFunctionMap(); 

//    cg.dump();
//    errs() << "------------traverse start----------\n";
//    traverse(cg.getRoot(), 0);
//    errs() << "------------traverse end----------\n";
    errs() << "------------traverseFunction start----------\n";
    traverseFunction(cg.getRoot(),0);
    errs() << "------------traverseFunction end----------\n";
    
    
    //    map<FunctionDecl*, FunctionDecl*> *callRelation[PACKAGE_NUM][PACKAGE_NUM];
    //    vector<FunctionDecl*> * rootFunctions[PACKAGE_NUM];
    for(int i = 0; i < PACKAGE_NUM; i ++){
        for(int j = 0; j < PACKAGE_NUM; j ++){
            callRelation[i][j] = new map<FunctionDecl*, FunctionDecl*>();
        }
    }
    
    for(int i = 0; i < PACKAGE_NUM; i ++){
        rootFunctions[i] = new vector<FunctionDecl*>();
    }
    
    //find the margins between each package pair
    errs() << "------------findMargin start----------\n";
    findMargin(cg.getRoot());
    errs() << "------------findMargin end----------\n";
    printMargin();
    printMarginDot();
    
    //first find all the callee functions
    vector<FunctionDecl*> callees;
    errs() << "------------findCallee start----------\n";
    findCallee(cg.getRoot(), callees);
    errs() << "------------findCallee end----------\n";
    //then find root callers from all function set
    findRootCaller(callees);
    printRootCaller();
    
    
    
    
    return result;
}

void findMargin(CallGraphNode * root){
    
	if(!root) return;
    
    
	string callerType = "";
    
    
    for(CallGraphNode::iterator it = root->begin(); it!= root->end(); ++it){
        
        FunctionDecl* callee = dyn_cast_or_null<FunctionDecl> ((*it)->getDecl());
        FunctionDecl* caller = dyn_cast_or_null<FunctionDecl>(root->getDecl());
        
        
        if(caller && callee){
            /* if (!callee->isThisDeclarationADefinition()) {
             for (FunctionDecl::redecl_iterator it2 = callee->redecls_begin(); it2 != callee->redecls_end(); ++it2) {
             if ((*it2)->isThisDeclarationADefinition()) {
             callee = *it2;
             break;
             }
             }
             }*/
        	if(!callee->isThisDeclarationADefinition())
        		continue;
            if(caller->getNameAsString ()== callee->getNameAsString())
                continue;
            else {
                if((callerType != "" || funcPackageMap.find(root->getDecl())!= funcPackageMap.end()) && funcPackageMap.find(callee) != funcPackageMap.end()){
                    if(callerType == "")
                        callerType = funcPackageMap.find(root->getDecl())->second;
                    
                    string calleeType = funcPackageMap.find((*it)->getDecl())->second;
                    
                    //errs()<<caller->getNameAsString() << " calls " << callee->getNameAsString() << "\n";
                    
                    
                    if(callerType != calleeType){
                        int callerIndex = getTypeIndex(callerType);
                        int calleeIndex = getTypeIndex(calleeType);
                        if(callerIndex == -1){
                            errs() << root->getDecl()->getASTContext().getSourceManager().getFilename(root->getDecl()->getSourceRange().getBegin());
                        }
                        if(calleeIndex == -1){
                            errs() << (*it)->getDecl()->getASTContext().getSourceManager().getFilename(root->getDecl()->getSourceRange().getBegin());
                        }
                        
                        if(callerIndex != -1 && calleeIndex != -1){
                            callRelation[callerIndex][calleeIndex]->insert(make_pair(caller, callee));
                        }
                    }
                }
            }
        }
        findMargin((*it));
    }
    
}

void findCallee(CallGraphNode *root, vector<FunctionDecl*> & callees){
	if(!root) return;
    
	for(CallGraphNode::iterator it = root->begin(); it!= root->end(); ++it){
		FunctionDecl* callee = dyn_cast_or_null<FunctionDecl> ((*it)->getDecl());
		FunctionDecl* caller = dyn_cast_or_null<FunctionDecl> (root->getDecl());
		if(caller && callee){
			if(caller->getNameAsString() == callee->getNameAsString())
				continue;
			else{
                
				/*if (!callee->isThisDeclarationADefinition()) {
                 for (FunctionDecl::redecl_iterator it2 = callee->redecls_begin(); it2 != callee->redecls_end(); ++it2) {
                 if ((*it2)->isThisDeclarationADefinition()) {
                 callee = *it2;
                 break;
                 }
                 }
                 }*/
				if(!callee->isThisDeclarationADefinition()){
					continue;
				}
                if (callee->getNameAsString()=="cyg_package_start") {
                    errs() << caller->getNameAsString();
                    errs() << "hehe";
                }
				if(funcPackageMap.find(callee) != funcPackageMap.end()){
                    
					bool newfunction = true;
					for(vector<FunctionDecl*>::iterator it3 = callees.begin(); it3 != callees.end(); ++it3){
						if((*it3)->getType().getAsString() == callee->getType().getAsString()
                           && (*it3)->getNameAsString() == callee->getNameAsString()){
							newfunction = false;
							break;
						}
                        
					}
					if(newfunction){
						callees.push_back(callee);
					}
				}
			}
		}
		findCallee(*it, callees);
	}
}

void findRootCaller(vector<FunctionDecl*> & callees){
	for(map<Decl*, string> :: iterator it = funcPackageMap.begin(); it != funcPackageMap.end(); ++it){
		FunctionDecl* function = dyn_cast_or_null<FunctionDecl>((*it).first);
		if(function){
			bool rootcaller = true;
			for(vector<FunctionDecl*>:: iterator it2 = callees.begin(); it2!= callees.end(); ++it2){
				if((*it2)->getNameAsString() == function->getNameAsString()){
					rootcaller = false;
					break;
				}
			}
			if(rootcaller){
				string typestring = (*it).second;
				if(typestring == "empty")
					errs()<< "empty package" << function->getNameAsString() << "\n";
				else{
					bool newfunction = true;
					vector<FunctionDecl*> * current = rootFunctions[getTypeIndex(typestring)];
					for(vector<FunctionDecl*>::iterator iter = current->begin(); iter!= current->end(); ++iter){
						if((*iter)->getNameAsString() == function->getNameAsString()){
							newfunction = false;
							break;
						}
					}
					if(newfunction){
						current->push_back(function);
					}
				}
			}
		}
        
	}
}

int getTypeIndex(string type){
	for(int i = 0; i < PACKAGE_NUM; i ++){
		if(keys[i] == type)
			return i;
	}
	return -1;
}


void printMargin(){
    std::ofstream margin("margin.txt");
    margin << "digraph \"Margin\"\n{";
    margin << "edge [fontname=\"Helvetica\",fontsize=\"10\",labelfontname=\"Helvetica\",labelfontsize=\"10\"];node [fontname=\"Helvetica\",fontsize=\"10\",shape=record];rankdir=\"LR\";";
	for(int i = 0; i < PACKAGE_NUM; i ++){
		for(int j = 0; j < PACKAGE_NUM; j ++){
			if(i == j)
				continue;
			errs()<< "-------------from package " << keys[i] << " to package " << keys[j] << "-------------\n";
			for(map<FunctionDecl*, FunctionDecl*>::iterator iter = callRelation[i][j]->begin(); iter != callRelation[i][j]->end(); iter ++){
				FunctionDecl* caller = iter->first;
				FunctionDecl* callee = iter->second;
                
				errs() << caller->getNameAsString() << " calls " << callee -> getNameAsString() << "\n";
                margin << caller->getNameAsString() << "->" << callee -> getNameAsString() << ";\n";
			}
		}
	}
    margin << "}";
}
void printMarginDot(){
    std::ofstream margin("margin.dot");
    margin << "digraph \"Margin\"\n{";
    margin << "edge [fontname=\"Helvetica\",fontsize=\"10\",labelfontname=\"Helvetica\",labelfontsize=\"10\"];node [fontname=\"Helvetica\",fontsize=\"10\",shape=record];rankdir=\"LR\";";
	for(int i = 0; i < PACKAGE_NUM; i ++){
        vector<FunctionDecl*> callers;
        vector<FunctionDecl*> callees;
		for(int j = 0; j < PACKAGE_NUM; j ++){
			if(i == j)
				continue;
			for(map<FunctionDecl*, FunctionDecl*>::iterator iter = callRelation[i][j]->begin(); iter != callRelation[i][j]->end(); iter ++){
				FunctionDecl* caller = iter->first;
				FunctionDecl* callee = iter->second;
                margin << caller->getNameAsString() << "->" << callee -> getNameAsString() << ";\n";
                callers.push_back(caller);
                callees.push_back(callee);
			}
            if (callers.empty()||callees.empty()) {
                continue;
            }
            margin << "subgraph cluster" << i << 0 << "{\n";
            if (funcPackageMap.find(*callers.begin())!=funcPackageMap.end()) {
                margin << "label = \"" << funcPackageMap.find(*callers.begin())->second << "\";";
            }
            for (vector<FunctionDecl*>::iterator it = callers.begin(); it != callers.end(); ++it) {
                margin << (*it)->getNameAsString() << "\n";
            }
            margin << "}\n";
            margin << "subgraph cluster" << j << 1 << "{\n";
            if (funcPackageMap.find(*callees.begin())!=funcPackageMap.end()) {
                margin << "label = \"" << funcPackageMap.find(*callees.begin())->second << "\";";
            }
            for (vector<FunctionDecl*>::iterator it = callees.begin(); it != callees.end(); ++it) {
                margin << (*it)->getNameAsString() << "\n";
            }
            margin << "}\n";
		}
	}
    margin << "}";
}


void printRootCaller(){
	for(int i = 0; i < PACKAGE_NUM; i ++){
		errs() << "-------------in package " << keys[i] << "---------\n";
		for(vector<FunctionDecl*>::iterator iter = rootFunctions[i]->begin(); iter != rootFunctions[i]->end(); iter ++){
			errs() << (*iter)->getNameAsString() << "\n";
		}
	}
}
