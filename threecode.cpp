ofstream tcode("threecode.txt");

vector<string> generateargs(ttnode *t);
vector<string> generatevars(ttnode *t);
vector<param> generatepars(ttnode *t);

symtable mipstable;

string newlabel()
{
	static int c = 0;
	stringstream ss;
	ss << ++c;
	return "label_" + ss.str();
}

string newtemp()
{
	stringstream ss;
	ss << ++tempco;
	string s = "temp_" + ss.str();
	mipstable.add_var(symbol(s));
	return mipstable.lookup(s);
}

string getstrl()
{
	static int c = 0;
	stringstream ss;
	ss << ++c;
	return "_.string_" + ss.str();
}

vector<string> breaks;
vector<string> continues;

string generatecode(ttnode *t)
{
	if(t->identifier == "variable_declaration")
	{
		vector<string> v = generatevars(t->second);
		for(int i=0;i<v.size();i++)
		{
			mipstable.add_var(symbol(v[i]));
			tcode << "variable_declaration " << v[i] << endl;
		}
		return def;
	}
	if(t->identifier == "function_declaration")
	{
		if(t->third != NULL)
		{
			vector<param> v = generatepars(t->second->first);
			stringstream ss;
			ss << v.size();
			tcode << "_"+t->item+"_."+ss.str() << ":" << endl;
			genmips("_"+t->item+"_."+ss.str());
			//tcode << v.size() << endl;
			mipstable.add_var(symbol(t->item+"_."+ss.str(),t_int,v));
			mipstable.add_scope();
			for(int i=0;i<v.size();i++)
			{
				mipstable.add_var(symbol(v[i].name));
				tcode << "parameter " << v[i].name << endl;
			}
			string a = generatecode(t->third);
			//tcode << "function done " << endl;
		}
		else
		{
			vector<param> v = generatepars(t->first->first);
			stringstream ss;
			ss << v.size();
			tcode << "_"+t->item+"_."+ss.str() << ":" << endl;
			genmips("_"+t->item+"_."+ss.str());
			mipstable.add_var(symbol(t->item+"_."+ss.str(),t_int,v));
			mipstable.add_scope();
			for(int i=0;i<v.size();i++)
			{
				mipstable.add_var(symbol(v[i].name));
			}
			string a = generatecode(t->second);	
		}
		mipstable.remove_scope();
		tcode << "return to caller" << endl;
		genmips(def,"exit");
		return def;
	}
	if(t->identifier == "main_function")
	{
		tcode << "main" << ":" << endl;
		genmips("main");
		mipstable.add_scope();
		string a = generatecode(t->first);
		mipstable.remove_scope();
		genmips(def,"exit");
		return def;
	}
	if(t->identifier == "variable")
	{
		return mipstable.lookup(t->first->item);
	}
	if(t->identifier == "statement")
	{
		if(t->item == "break")
		{
			tcode << "goto " << breaks.back() << endl;
			genmips(def,"goto",breaks.back());
		}
		else if(t->item == "continue")
		{
			tcode << "goto " << continues.back() << endl;
			genmips(def,"goto",continues.back());
		}
		else
			string a = generatecode(t->first);
		return def;
	}
	if(t->identifier == "condition_stat")
	{
		string start = newlabel();
		string end = newlabel();
		string a = generatecode(t->first);
		tcode << "if " << a << " goto " << start << endl;
		genmips(def,"if",a,def,start);
		tcode << "goto " << end << endl;
		genmips(def,"goto",end);
		tcode << start << ":" << endl;
		genmips(start);
		mipstable.add_scope();
		string b = generatecode(t->second);
		mipstable.remove_scope();
		tcode << end << ":" << endl;
		genmips(end);
		if(t->item == "op")
		{
			mipstable.add_scope();
			string c = generatecode(t->third);
			mipstable.remove_scope();
		}
		return def;
	}
	if(t->identifier == "for_loop")
	{
		string start = newlabel();
		string middle = newlabel();
		string con = newlabel();
		string end = newlabel();
		string a = generatecode(t->first);
		string b = generatecode(t->second);
		string temp = newtemp();
		breaks.push_back(end);
		continues.push_back(con);
		tcode << a << " = 0" << endl;
		genmips(def,"+","0","0",a);
		tcode << start << ":" << endl;
		genmips(start);
		tcode << temp << " = " << a << " < " << b << endl;
		genmips(def,"<",a,b,temp);
		tcode << "if " << temp << " goto " << middle << endl;
		genmips(def,"if",temp,def,middle);
		tcode << "goto " << end << endl;
		genmips(def,"goto",end);
		tcode << middle << ":" << endl;
		genmips(middle);
		mipstable.add_scope();
		string c = generatecode(t->third);
		mipstable.remove_scope();
		tcode << con << ":" << endl;
		genmips(con);
		tcode << a << " = " << a << " + 1" << endl;
		genmips(def,"+",a,"1",a);
		tcode << "goto " << start << endl;
		genmips(def,"goto",start);
		tcode << end << ":" << endl;
		genmips(end);
		breaks.pop_back();
		continues.pop_back();
		return def;
	}
	if(t->identifier == "while_loop")
	{
		string start = newlabel();
		string middle = newlabel();
		string end = newlabel();
		breaks.push_back(end);
		continues.push_back(start);
		tcode << start << ":" << endl;
		genmips(start);
		string a = generatecode(t->first);
		tcode << "if " << a << " goto " << middle << endl;
		genmips(def,"if",a,def,middle);
		tcode << "goto " << end << endl;
		genmips(def,"goto",end);
		tcode << middle << ":" << endl;
		genmips(middle);
		mipstable.add_scope();
		string b = generatecode(t->second);
		mipstable.remove_scope();
		tcode << "goto " << start << endl;
		genmips(def,"goto",start);
		tcode << end << ":" << endl;
		genmips(end);
		breaks.pop_back();
		continues.pop_back();
		return def;
	}
	if(t->identifier == "return_stat")
	{
		if(t->item == "op")
		{
			//tcode << "return expression" << endl;
			string a = generatecode(t->second);
			tcode << "return " << a << endl;
			genmips(def,"return",a);
		}
		else
		{
			tcode << "return " << endl;
		}
		genmips(def,"exit");
		return def;
	}
	if(t->identifier == "read")
	{
		string a = generatecode(t->first);
		tcode << "read " << a << endl;
		genmips(def,"read",a);
		return a;
	}
	if(t->identifier == "write")
	{
		string a = generatecode(t->first);
		tcode << "write " << a << endl;
		genmips(def,"write",a);
		return a;
	}
	if(t->identifier == "putstring")
	{
		string s = getstrl();
		pair<string,string> p = make_pair(s,t->first->item);
		lit_strings.push_back(p);
		tcode << "print " << s << endl;
		genmips(def,"puts",s);
	}
	if(t->identifier == "expression")
	{
		string b;
		if(t->item == "=")
		{
			string a = generatecode(t->second);
			b = generatecode(t->first);
			tcode << b << " = " << a << endl;
			genmips(def,"+",a,"0",b);
		}
		else
		{
			b = generatecode(t->first);
		}
		string ret = newtemp();
		tcode << ret << " = " << b << endl;
		genmips(def,"+",b,"0",ret);
		return ret;
	}
	if(t->identifier == "logic_expression")
	{
		if(t->item == "or")
		{
			string a = generatecode(t->second);
			string b = generatecode(t->first);
			string ret = newtemp();
			tcode << ret << " = " << b << " || " << a << endl;
			genmips(def,"||",b,a,ret);
			return ret;
		}
		else
		{
			return generatecode(t->first);
		}
	}
	if(t->identifier == "and_expression")
	{
		if(t->item == "and")
		{
			string a = generatecode(t->second);
			string b = generatecode(t->first);
			string ret = newtemp();
			tcode << ret << " = " << b << " && " << a << endl;
			genmips(def,"&&",b,a,ret);
			return ret;
		}
		else
		{
			return generatecode(t->first);
		}
	}
	if(t->identifier == "relation_expression")
	{
		if(t->item == "op")
		{
			string a = generatecode(t->third);
			string b = generatecode(t->first);
			string c = generatecode(t->second);
			string ret = newtemp();
			tcode << ret << " = " << b << " " << c << " " << a << endl;
			genmips(def,c,b,a,ret);
			return ret;
		}
		else
		{
			return generatecode(t->first);
		}
	}
	if(t->identifier == "simple_expression")
	{
		if(t->item == "op")
		{
			string a = generatecode(t->third);
			string b = generatecode(t->first);
			string c = generatecode(t->second);
			string ret = newtemp();
			tcode << ret << " = " << b << " " << c << " " << a << endl;
			genmips(def,c,b,a,ret);
			return ret;
		}
		else
		{
			return generatecode(t->first);
		}
	}
	if(t->identifier == "mul_expression")
	{
		if(t->item == "op")
		{
			string a = generatecode(t->third);
			string b = generatecode(t->first);
			string c = generatecode(t->second);
			string ret = newtemp();
			tcode << ret << " = " << b << " " << c << " " << a << endl;
			genmips(def,c,b,a,ret);
			return ret;
		}
		else
		{
			return generatecode(t->first);
		}
	}
	if(t->identifier == "unary_expression")
	{
		if(t->item == "op")
		{
			string a = generatecode(t->second);
			string b = generatecode(t->first);
			string ret = newtemp();
			tcode << ret << " = " << b << a << endl;
			genmips(def,b,"0",a,ret);
			return ret;
		}
		else
		{
			return generatecode(t->first);
		}
	}
	if(t->identifier == "climax")
	{
		return generatecode(t->first);
	}
	if(t->identifier == "constants")
	{
		return t->first->item;
	}
	if(t->identifier == "call")
	{
		//evaluate arguments
		vector<string> v = generateargs(t->first->first);
		stringstream ss;
		ss << v.size();
		vector<string> pars = mipstable.getparams(t->item+"_."+ss.str());
		tcode << "push return address" << endl;
		genmips(def,"pushreturn");
		//backup variables
		vector<string> backvars = mipstable.backup();
		for(int i=0;i<backvars.size();i++)
		{
			tcode << "push " << backvars[i] << endl;
			genmips(def,"push",backvars[i]);
		}
		//set arguments
		//tcode << pars.size() << " " << v.size() << endl;
		for(int i=0;i<v.size();i++)
		{
			//tcode << "argument " << v[i] << endl;
			tcode << "copy " << v[i] << " to " << pars[i] << endl;
			genmips(def,"copy",v[i],pars[i]);
		}
		//call function
		tcode << "call " << "_"+t->item+"_."+ss.str() << endl;
		genmips(def,"call","_"+t->item+"_."+ss.str());
		//restore variables
		for(int i = backvars.size()-1;i>=0;i--)
		{
			tcode << "pop " << backvars[i] << endl;
			genmips(def,"pop",backvars[i]);
		}
		mipstable.restore(backvars);
		tcode << "pop return address" << endl;
		genmips(def,"popreturn");
		string ret = newtemp();
		tcode << "assign return value in " << ret << endl;
		genmips(def,"restorereturn",ret); 
		return ret; 
	}
	if(t->identifier == "op1")
	{
		return t->first->item;
	}
	if(t->identifier == "op2")
	{
		return t->first->item;
	}
	if(t->identifier == "op3")
	{
		return t->first->item;
	}
	if(t->identifier == "unary_op")
	{
		return t->first->item;
	}
	else
	{
		if(t->first != NULL)
			generatecode(t->first);
		if(t->second != NULL)
			generatecode(t->second);
		if(t->third != NULL)
			generatecode(t->third);
		return def;
	}
}

vector<string> generateargs(ttnode *t)
{
	vector<string> v;
	if(t->identifier != "epsilon")
	{
		if(t->second == NULL)
		{
			v.push_back(generatecode(t->first));
		}
		else
		{
			v = generateargs(t->first);
			v.push_back(generatecode(t->second));
		}
	}
	return v;
}

vector<string> generatevars(ttnode *t)
{
	vector<string> v;
	if(t->second == NULL)
	{
		v.push_back(t->first->item);
	}
	else
	{
		v = generatevars(t->first);
		v.push_back(t->second->item);
	}
	return v;
}

vector<param> generatepars(ttnode *t)
{
	vector<param> v;
	if(t->identifier != "epsilon")
	{
		if(t->second == NULL)
		{
			//v.push_back(param(t->first->second->first->item));
			v.push_back(param(t->first->second->first->item,t->first->type));
		}
		else
		{
			v = generatepars(t->first);
			v.push_back(param(t->second->second->first->item,t->second->type));
		}
	}
	return v;
}