#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <regex>

using namespace std;
#define VNAME(value) (#value)

enum type{
	NUM,
	STRING,
	LIST_EMPTY,
	LIST_WARN,
	UNDEFINE,
	UNKNOWN   //this program unknown, not undefine
};

enum mark{
	OK,
	ERROR, 
	WARN,
	UNDEF
};

typedef pair<string, type> var;



bool isList(type t){
	return t==LIST_EMPTY || t==LIST_WARN;
}

void reSpace(string& in){
	/* remove space */
	string out="";
	for(auto s: in){
		if (s !=' ') out += s; 
	}
	in = out;
}


void reComm(string& in){
	/* remove space */
	string out="";
	for(auto s: in){
		if (s !=':') out += s; 
	}
	in = out;
}

//remove if from begining
void stripIf(string& line){
	if(line.substr(0,2)=="if") 
		line=line.substr(2);
	else if (line.substr(0,4)=="elif") 
		line=line.substr(4);
}

//regular expression with embedding
string SIM_OPRAND = "((\\s*)((\\w|\"|\\.)+)(\\s*))";  //operand, could be string, instant num, var
string LIST = "(\\[\\])|(list\\(\\))";   //[], list()
string LIST_VAR = "((\\s*)(\\w+)\\[\\d+\\](\\s*))";  //list member
string SIM_FUNC = "(\\w+\\(((\\w|,|\")+)\\))";   //f(a,b,"3")
string EQ = "(\\s*)=(\\s*)"; 
string ADD = "(\\s*)\\+(\\s*)";
string MUL  = "(\\s*)\\*(\\s*)";
string OP = "(\\+|\\*)";
string BYPASS = "(import)|(else)|(main)|(\\/)|#";
string IF="^((if)|(elif))(.+)";
string DEF= "^(def)(.+)";
string RETURN = "^(return)(.+)";
string STR = "(\".*\")|(\'.*\')";
string NUMB = "(\\d|\\.)+";
string SIM_VAR = "((\\s*)(\\w+)(\\s*))";  //simple var, no list
string VAR = "("+LIST_VAR+"|"+SIM_VAR+")";  //simple var or list member var. use as left side
string OPRAND = "\\(?("+SIM_OPRAND+"|"+LIST_VAR+"|"+SIM_FUNC+")\\)?(?!:)";   //use as right side
string EXP = "(.+)(\\*|\\+)+(.+)";  //expression, +/* at least once, at least one operation
string BRACE = "\\((.*)\\)"; 			// () strcture
string FUNC = "(\\w+)\\.(\\w+)\\((.*)\\)"; //name.func()  structure


//expression type
string VAR_ONLY = OPRAND; //a,  f(a,b)
string FUNC_ONLY = FUNC;  //name.func()
string OP_ONLY = OPRAND+"("+OP+OPRAND+")+";  //a+b
string SIM_DEC_ONLY = SIM_VAR+EQ+"(("+STR+")|("+NUMB+")|("+LIST+")|("+VAR+")|("+SIM_FUNC+"))";   //a=1.5, on list element on left side
string EXP_ONLY = VAR+EQ+EXP; //a=c+d
string LIST_ASSIGN_ONLY = LIST_VAR+EQ+SIM_OPRAND; //m[3]=4

//var declaration
string VAR_EQ_STRING 	= VAR+EQ+STR;
string VAR_EQ_NUM 		= VAR+EQ+NUMB;
string VAR_EQ_LIST 		= VAR+EQ+LIST;
string VAR_EQ_VAR 		= VAR+EQ+VAR;

//expression assign values
string VAR_EQ_EXP = VAR+EQ+EXP;
string VAR_EQ_FUNC = VAR+EQ+FUNC; 
string LIST_EQ_VAR = LIST_VAR+EQ+OPRAND; 
string LIST_EQ_EXP = LIST_VAR+EQ+EXP;

//operation
string ADD_OP = OPRAND+ADD+OPRAND;
string MUL_OP = OPRAND+MUL+OPRAND;
string ADD_BRACE = OPRAND+ADD+BRACE;
string MUL_BRACE = OPRAND+MUL+BRACE;

	
vector<string> line_type = {
	VAR_ONLY,FUNC_ONLY,OP_ONLY,EXP_ONLY,SIM_DEC_ONLY,LIST_ASSIGN_ONLY, IF, DEF, RETURN
};	 
vector<string> line_type_name = {
	"VAR_ONLY","FUNC_ONLY","OP_ONLY","EXP_ONLY","SIM_DEC_ONLY","LIST_ASSIGN_ONLY","IF", "DEF", "RETURN"
};	 


vector<string>	dec_type = {
	VAR_EQ_STRING, VAR_EQ_NUM, VAR_EQ_LIST, VAR_EQ_VAR
};
vector<string>	dec_type_name = {
	"VAR_EQ_STRING", "VAR_EQ_NUM", "VAR_EQ_LIST", "VAR_EQ_VAR"
};

vector<string> exp_type = {
	VAR_EQ_EXP, VAR_EQ_FUNC, LIST_EQ_VAR, LIST_EQ_EXP 
};
vector<string> exp_type_name = {
	"VAR_EQ_EXP", "VAR_EQ_FUNC", "LIST_EQ_VAR", "LIST_EQ_EXP" 
};

vector<string> op_type = {
	ADD_OP, MUL_OP, ADD_BRACE, MUL_BRACE
};
vector<string> op_type_name = {
	"ADD_OP", "MUL_OP", "ADD_BRACE", "MUL_BRACE"
};



vector<var> var_list;  //store all the vars
type type_check(string ss);
void update_type(string var_name, type t);
int func_def=0;
string func_name="";
/////*********** funcitons start ************* /////
void op_split(string ss, vector<string>& op_list, vector<char>& sign_list){
	string temp = "";
	for(auto s:ss){
		if(s=='+'||s=='*'||s=='('||s==')'||s=='='||s==','){
			if (!temp.empty()) op_list.push_back(temp);
			temp = "";
			if(s=='+' || s=='*'||s=='='){
				sign_list.push_back(s);
			}
		}
		else temp += s;
	}
	if (!temp.empty()) op_list.push_back(temp);
	cout<<"all oprands:";
	for (auto op: op_list){cout<<op<<" ";}
}


string get_func_name(string line){
	string name="";
	line = line.substr(3); //remove def
	string temp = "";
	for(auto s:line){
		if(s=='('){
			return temp;
		}
		temp +=s;
	}
	return "STEPHEN";
}

type return_check(string line){
	for(auto i:line){
		if (i=='*') return NUM;
	}
	return UNKNOWN;
}

//error op
mark error_op(vector<string> op_list, vector<char> sign_list){
	//string multiply check, list multiply check
	int cn=0;
	string op_left="";
	string op_right ="";
	for(auto s: sign_list){
		if(s=='*'){
			op_left = op_list[cn];
			op_right = op_list[cn+1];
			
			if (type_check(op_left)==STRING && type_check(op_right)==STRING){
				cout<<"string multiply error ";
				return ERROR;
			}
			if (isList(type_check(op_left)) && isList(type_check(op_right))){
				cout<<"List multiply error ";
				return ERROR;
			}
		}
		cn++;		
	}
	return OK;
}


void rm_unknown(vector<string>& op_list){
	auto num = op_list.begin();
	for(auto op:op_list){
		if(op=="exp"){
			op_list.erase(num);
			cout<<"remove op:"<<op<<";";			
		}
		else if (type_check(op)==UNKNOWN){
			op_list.erase(num);
			cout<<"remove unknown op:"<<op<<";";
		}
		num++;
	}
}

//a+b  
mark op_check(string ss){
	vector<string> op_list;
	vector<type> type_list;
	vector<char> sign_list;
	op_split(ss, op_list, sign_list);
	cout<<"op num:"<<op_list.size()<<";";
	
	rm_unknown(op_list);
	mark out = error_op(op_list, sign_list);
	if (out==ERROR) return ERROR;
	
	for(auto op: op_list){
		type ty = type_check(op);
		if(ty==UNDEFINE) return UNDEF;
		if(ty==LIST_WARN) return WARN;
		for(auto tt: type_list){
			if (tt!=ty) return ERROR;
		}
		type_list.push_back(ty);
	}
	return OK;
}

//c = a+b
mark exp_check(string ss){
	vector<string> op_list;
	vector<type> type_list;
	vector<char> sign_list;
	op_split(ss, op_list, sign_list);
	int num=0;
	string var_name="";
	cout<<"exp num:"<<op_list.size()<<";";
	
	rm_unknown(op_list);
	mark out = error_op(op_list, sign_list);
	if (out==ERROR) return ERROR;
	
	for(auto op: op_list){
		if(num==0) {
			var_name = op;
		}
		else{
			type ty = type_check(op);
			if(ty==UNDEFINE) return UNDEF;
			if(ty==LIST_WARN) return WARN;
			for(auto tt: type_list){
				if (tt!=ty) return ERROR;
			}
			type_list.push_back(ty);
		}
		num++;
	}
	if(!type_list.empty()) {
		type var_type = type_check(var_name);
		if(var_type == UNDEFINE || var_type ==LIST_EMPTY){
			cout<<"type assign for "<<var_name<<" to "<<type_list[0]<<"; ";  //type assign
			update_type(var_name, type_list[0]);
			return OK;	
		}
		else if(var_type==LIST_WARN){
			return WARN;
		}
		else if (var_type != type_list[0]) {
			cout<<"type change for "<<var_name<<" to "<<type_list[0]<<"; ";  //type change
			update_type(var_name, type_list[0]);
			return WARN;
		}
		else return OK;
	}
	return OK;
}


bool hasFunction(string line){
	regex pat(SIM_FUNC);
	smatch res;
	bool search=false;
	search = regex_search(line, res, pat);
	if(search){
		cout<<"fincution inclued in this line!";
		return true;
	}
	return false;
}



void update_type(string var_name, type t){
	int find=0;
	int num=0;
	for(auto vv:var_list){
		if(vv.first==var_name) {
			var_list[num].second = t;
			find=1;
			cout<<"update var "<<var_name<<" to type "<<t<<"; ";
			return;
		}
		num++;
	}
	if (find==0) {
		cout<<"insert new var:"<<var_name<<" with type "<<t<<"; ";
		var_list.push_back({var_name, t});
	}	
}


void split_by_com(string ss, vector<string>& slist){
	string temp = "";
	for(auto s:ss){
		if(s==','){
			if (!temp.empty()) slist.push_back(temp);
			temp = "";
		}
		else temp += s;
	}
	if (!temp.empty()) slist.push_back(temp);
}

void test_reg(string ss, string t){
	regex pat(t);
	smatch res;
	bool search=false;
	search = regex_search(ss, res, pat);
	cout<<"try pattern: "<<t<<endl;
	if (search) {
		cout<<"--match!!"<<endl;
		for (auto i:res){
			cout<<"		res:"<<i<<endl;
		}
	}
}

type type_check(string ss){
	cout<<"check type input:"<<ss<<";";
	smatch res;
	string int_p = "\\d+";  //num
	string str_p = "\"(.+)\"";  //string
	string var_p = "\\w+"; //var
	string list_ele_p = LIST_VAR; //list_var
	regex pat(int_p);
	if(regex_match(ss, res, pat)) {cout<<"type 0; "; return NUM;}
	pat = str_p;
	if(regex_match(ss, res, pat)) {cout<<"type 1; "; return STRING;}
	pat = var_p;
	if(regex_match(ss, res, pat)) {
		string var_name = res.str(0);
		for(auto vv:var_list){
			if (vv.first==var_name) {cout<<"type "<<vv.second<<"; "; return vv.second;}
		}
		cout<<"type check fail, unknown var;";
		return UNDEFINE;
	}
	
	pat = list_ele_p;
	if(regex_match(ss, res, pat)) {
		cout<<"type list element; "; 
		string var_name = res.str(3);
		for(auto vv:var_list){
			if (vv.first==var_name) return vv.second;
			else{
				cout<<"type check fail, unknown list ele;";
				return UNDEFINE;
			}
		}
	
	}
	if (ss=="list()") {cout<<"type empty list; "; return LIST_EMPTY;}
	if (ss=="[]") 	{cout<<"type empty list; "; return LIST_EMPTY;}
	cout<<"Not exist！"<<endl;
	return UNDEFINE;
}

mark process(string line){
	bool match = false;
	bool search = false;
	mark out=OK;
	smatch res;
	int num=0;
	int cnt=0;

	
	//test bypass first
	reSpace(line);
	regex pat(BYPASS);
	search = regex_search(line, res, pat);
	if(search){
		cout<<"line: bypass, line:"<<line;
		return OK;
	} 
	//test other patterns
	for(auto t:line_type){
		if(func_def==1 && num<=7) {
			num++;
			continue;
		}
		regex pat(t);
		cout<<"try patter name: "<<line_type_name[num]<<",pattern: "<<t<<endl;
		match = regex_match(line, res, pat);
		if (match) {
			cout<<"--match!! patter name: "<<line_type_name[num]<<",pattern: "<<t<<endl;
			switch(num){
				case 6:{ //if statement
					cout<<"line: if statement";
					reComm(line);
					stripIf(line);
					if (op_check(line)== ERROR) return WARN;
				}
				break;
				case 7:{ //def statement
					cout<<"line: def statement";
					func_def =1;
					func_name = get_func_name(line);
					cout<<"get funciton name:"<<func_name<<";";
					return OK;
				}
				break;
				case 8:{ //return statement
					cout<<"line: return statement";
					func_def =0;
					type ty = return_check(line);
					cout<<"this funciton type is:"<<ty<<";";
					var_list.push_back({func_name, ty});
					func_name="";
					return OK;					
				}
				break;
				
				case 0: {//var only 
					cout<<"line: var only, res:";
					bool has_function = hasFunction(line);
					if (has_function){
						return op_check(line);
					}
					else {
						cnt=0;
						for(auto i:res) {cout<<cnt<<"->"<<i<<";";cnt++;}
						string var_name = res.str(1);
						type tt = type_check(var_name);
						if(tt==UNDEFINE) return UNDEF;
						return OK;
					}
				}
				break;				
				case 1: {//func only
					cout<<"line: func only, res:";
					cnt=0;
					for(auto i:res) {cout<<cnt<<"->"<<i<<";";cnt++;}
					string class_name = res.str(1);
					string inside_brace = res.str(3);
					//reSpace(inside_brace);
					vector<string> slist;
					split_by_com(inside_brace, slist); //split all parameters
					type tt;
					if (!slist.empty()) {
						tt = type_check(*slist.rbegin());
					}
					int find=0;
					int num=0;
					for(auto vv: var_list){   //update type
						if (vv.first==class_name && vv.second==tt) {find=1; return OK;}
						if (vv.first==class_name && vv.second!=tt && (vv.second==LIST_EMPTY||vv.second==UNDEFINE)) //type assign
						{cout<<"list assign type;"<<tt<<"; "; var_list[num].second=tt;find=1; return OK;}
						if (vv.first==class_name && vv.second!=tt && vv.second!=UNDEFINE)   //change type
						{cout<<"list type change;"<<tt<<"; "; var_list[num].second=LIST_WARN;find=1; return WARN;}
						num++;
					}
					if(find==0) {  //insert new var
						var_list.push_back({class_name, tt});
						cout<<"insert new var:"<<class_name<<","<<tt<<",var_list len "<<var_list.size()<<endl; return UNDEF;
					}
					return OK;					
				}
				break;	
				case 2:  {//op only
					cout<<"line: op only, res:";
					cnt=0;
					for(auto i:res) {cout<<cnt<<"->"<<i<<";";cnt++;}
					//reSpace(line);
					return op_check(line);
				}
				break;	
				case 4: {//simple dec only
					cout<<"line: simple dec only, res:";
					bool has_function = hasFunction(line);
					if (has_function){
						return exp_check(line);
					}
					else {
						cnt=0;
						for(auto i:res) {cout<<cnt<<"->"<<i<<";";cnt++;}
						string var_name = res.str(1);
						string right_name = res.str(7);
						//reSpace(right_name);
						type tt = type_check(right_name);
						if(tt==UNDEFINE) {
							var_list.push_back({var_name, tt});
							return UNDEF;
						}
						int find = 0;
						int num=0;
						for(auto vv: var_list){   //update type
							if (vv.first==var_name && vv.second!=tt && (vv.second== UNDEFINE | vv.second== LIST_EMPTY)) //assign type
							{cout<<"type assign:"<<tt<<";"; var_list[num].second=tt; find=1;return OK;}						
							if (vv.first==var_name && vv.second!=tt && vv.second!= UNDEFINE && vv.second!= LIST_EMPTY) //change type
							{cout<<"type change:"<<tt<<";"; var_list[num].second=tt; find=1;return WARN;}
							if (vv.first==var_name && vv.second==tt)
							{find=1;return OK;} 
							num++;
						}
						if(find==0) {  //insert new var
							var_list.push_back({var_name, tt});
							cout<<"insert new var:"<<var_name<<","<<tt<<",var_list len "<<var_list.size()<<endl;
						}
						return OK;
					}
				}
				break;	
				case 3: {  //exp only
					cout<<"line: exp only, res:";
					cnt=0;
					for(auto i:res) {cout<<cnt<<"->"<<i<<";";cnt++;}
					//reSpace(line);
					return exp_check(line);
				}
				break;	
				case 5: {//list assign only
					cout<<"line: list assign only, res:";
					cnt=0;
					for(auto i:res) {cout<<cnt<<"->"<<i<<";";cnt++;}
					
					string var_name = res.str(3);
					string right_name = res.str(7);
					//reSpace(right_name);
					type tt = type_check(right_name);
					int find = 0;
					int num=0;
					for(auto vv: var_list){   //update type
						if (vv.first==var_name && vv.second==tt) 
						{find=1; return OK;}
						if (vv.first==var_name && vv.second!=tt && vv.second==LIST_EMPTY)  //type assign
						{cout<<"list type assign:"<<tt<<"; ";var_list[num].second=tt; find=1; return OK;}
						if (vv.first==var_name && vv.second!=tt && vv.second!=LIST_EMPTY) //type change
						{cout<<"list type change;"<<tt<<"; ";var_list[num].second=tt; find=1; return WARN;}
						num++;
					}
					if(find==0) {  //insert new var
						var_list.push_back({var_name, tt});
						cout<<"insert new var:"<<var_name<<","<<tt<<",var_list len "<<var_list.size()<<endl;
					}
					return OK;
				}
				break;	

				default: 
					cout<<"unrecognized line! string:"<<line<<endl;
					return OK;
			}
		}
		num++;
	}
	cout<<"no match for this line"<<endl;
	return OK;
}


int count_space(string line){
	int cnt=0;
	for(auto i:line){
		if (i==' ') cnt++;
		else return cnt;
	}
	return cnt;
}


int main(int argc, char* argv[]){
	string cmd=argv[1];
	auto pos = cmd.find('=');
	cmd = cmd.substr(pos+1);
	cout<<"input file is:"<<cmd<<endl;
	
	ifstream infile(cmd);
	string line;
	string tmp="";
	for(auto s:cmd){
		if(s=='.') cmd=tmp;
		else tmp += s;
	}
	ofstream ofile(cmd+".out");
	while(getline(infile, line)){
		cout<<endl;
		
		cout<<"line:"<<line<<endl;

		if (!line.empty() && line[line.size() - 1] == '\r')
    		line.erase(line.size() - 1);

		//string patt = "((\\s*)(\\w+)(\\s*))(\\s*)=(\\s*)((\\s*)(\\w+)(\\s*))";
		//string patt = EXP;
		//test_reg(line, patt);
		int space = count_space(line);
		mark mm;
		if(!line.empty())
			mm = process(line);
		else 
			mm = OK;
		string out_line = "";
		switch(mm){
			case OK: 
				ofile<<line<<endl; 
				break;
			case ERROR: {
				for(int i=0; i<space; i++){
					out_line += " ";
				}
				out_line +="#error";
				ofile<<out_line<<endl;
				ofile<<line<<endl;
				}
				break;
			case WARN:{
				for(int i=0; i<space; i++){
					out_line += " ";
				}
				out_line +="#warning";
				ofile<<out_line<<endl;
				ofile<<line<<endl;
				}
				break;
			case UNDEF:{
				for(int i=0; i<space; i++){
					out_line += " ";
				}
				out_line +="#undefined";
				ofile<<out_line<<endl;
				ofile<<line<<endl;		
				}
				break;
		}
		cout<<endl;
	}
	infile.close();
	ofile.close();
	return 1;
}
