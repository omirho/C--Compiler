enum Type
{
    t_none,
    t_int,
    t_float,
	t_bool,
    t_func
};

class param
{
	public:
		Type type;
		string name;
		param(string nm): name(nm), type(t_int) {}
		string genKey()
		{
			return name + "." + "func";
		}
};

class symbol
{
public:

    std::string name, val;
    Type type, ret_type;
	int scope, param_count;
	vector<param> paras;
    symbol(string nm,  Type t, int Scope): name(nm), scope(Scope), type(t) {}
    symbol(string nm,  Type t, Type rt, int Scope): name(nm), scope(Scope), type(t), ret_type(rt) {}
    symbol(string nm): name(nm), val(string()), type(t_int) {}
	string genKey()
	{
		string Types[] = {"none", "int", "float", "bool", "func"};
		stringstream ss;
		ss << scope;
		return name + "." + Types[type] + "." + ss.str();
	}
	symbol(string nm, Type t, vector<param> p): name(nm), paras(p), type(t), val(string()), param_count(p.size()) {}
};

class symtable
{
    int scope;
    vector < map < string, symbol > > table;
public:
    symtable(): scope(1) { table.push_back(map<string,symbol>());}

    string lookup(string id)
    {
        for(int i = scope; i > 0; --i)
            if(table[i-1].count(id))
                return table[i-1].find(id)->second.genKey();
        return string();
    }

    int add_scope()
    {
        table.push_back(map<string,symbol>());
        scope++;
    }

    int remove_scope()
    {
        table.pop_back();
        scope--;
    }

	void add_var(symbol s)
	{
		s.scope = scope;
		table[scope-1].insert(make_pair(s.name, s));
	}
	
	vector<string> getparams(string id)
	{
		symbol func = table[0].find(id)->second;
		vector<string> r;
		for(int i = 0; i < func.param_count; ++i)
		{
			r.push_back(symbol (func.paras[i].name, func.paras[i].type, func.scope + 1).genKey());
		}
		return r;
	}
	
	vector<string> backup()
	{
		vector<string> back_var;
		for(int i = 2; i <= scope; i++)
			for(map<string,symbol>::iterator it = table[i-1].begin(); it != table[i-1].end(); ++it)
				back_var.push_back(it -> second.genKey());
		for(int i = scope; i > 1; i--)
			table.pop_back();
		scope = 1;
	}
	
	Type getType(string s)
	{
			string Types[] = {"none", "int", "float", "bool", "func"};
			for(int i = 0; i < 5; ++i)
			{
				if(s == Types[i])
					return static_cast<Type>(i);
			}
			return t_none;
	}
	
	void restore(vector<string> back_var)
	{
		vector<symbol> vs;
		string name, type;
		int _scope, max_scope = -1;
		for(int i = 0; i < back_var.size(); ++i)
		{
			replace(back_var[i].begin(), back_var[i].end(), '.', ' ');
			stringstream ss(back_var[i]);
			ss >> name >> type >> _scope;
			max_scope = max(max_scope, _scope);
			vs.push_back(symbol(name, getType(type), _scope));
		}
		for(int i = 1; i < max_scope; ++i)
			 table.push_back(map<string,symbol>());
		for(int i = 0; i < vs.size(); i++)
			table[vs[i].scope - 1].insert(make_pair(vs[i].name,vs[i]));
			
		return back_var;
	}
};
