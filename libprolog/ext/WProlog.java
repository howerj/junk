//////////////////////////////////////////////////////
//
// Prolog in Java
//
// Michael Winikoff
//
// Version 1.0: 27/9/96 - 16/10/96
//
// Version 1.1: 20 January 1997
// 
// Version 1.2:  2 April 1997
//
// Version 1.2.1: 19 August 1999 (fixed bug with unification of variable
//		  to itself.
//
//////////////////////////////////////////////////////


// package wprolog;

import java.lang.*;
import java.applet.*;
import java.applet.AppletContext;
import java.awt.*;
import java.applet.Applet;
import java.util.*;
import java.io.*;
import java.net.*;
import java.lang.Runtime;

	
/** Simple frame to enable us to execute an Applet as a standalone.
* @author Michael Winikoff
*/
class AppletFrame extends Frame {

	static public AppletContext ac;	

	public AppletFrame() {
		super("W-Prolog 1.2.1");

		MenuBar menubar = new MenuBar();
		Menu file = new Menu("File",false);
		menubar.add(file);
		file.add("Quit");
		this.setMenuBar(menubar);

		Applet applet = new WPrologGUI(this);

		this.add("Center",applet);
		// width, height
		this.resize(432,559);
		this.show();

		applet.init();
		applet.start();
	}

	public boolean handleEvent(Event e) {
                if (e.id == Event.WINDOW_DESTROY) {
                // System.out.println("handleevent: quit");
                        if (WPrologGUI.inapplet) dispose(); 
                        else System.exit(0);
                }
		if (e.target instanceof MenuItem) {
			if (WPrologGUI.inapplet) dispose();
			else System.exit(0);
		}
		return false;
	}
}

/**
 * A Simple Prolog interpreter in Java.
 * @version 1.2.1 19 August 1997
 * @see WPrologGUI
 * @author Michael Winikoff 
 *	<a href="mailto:winikoff@acm.org"><em>winikoff@acm.org</em></a> 
 *	<a href="http://welcome.to/winikoff"><em>http://welcome.to/winikoff"</em></a>
*/

public class WProlog extends Applet {
	static private Button button;
	static public AppletContext ac;

    public void init() { setup(); }

    public static void main(String argv[]) { 
	WPrologGUI.inapplet = false;
	Frame frame = new AppletFrame();
    }

    public void setup() {
        GridBagConstraints c = new GridBagConstraints();

        GridBagLayout gridbag = new GridBagLayout();

        setLayout(gridbag);

        setFont(new Font("Dialog", Font.PLAIN, 12));

        // Add top item - query label
        c.gridx = 0; c.gridy = 0;
        c.gridwidth = 1;
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.fill = GridBagConstraints.BOTH;

        button = new Button("Start W-Prolog");
        gridbag.setConstraints(button,c);
        add(button);

	ac = getAppletContext();

        validate();
    }

    public void start() { }

    public boolean action(Event e, Object arg) {
        if (e.target == button) {
		Frame frame = new AppletFrame();
		return true;
	}
	return false;
    }
}
	

/** This class sets up the user interface.
* @author Michael Winikoff
*/
class WPrologGUI extends Applet {

    private Frame parent;

    private TextField querytf; private Button doquery;

    public TextArea program;
    public static TextArea results;

    public TextField filename;
    private Button load,save;
    private Checkbox occurcheck, trace;
    private Button more, clear, benchmark;
    private Button prog1, prog2, prog3, about;
/** This is used to store the Engine which is running the current query.
@see Engine
*/
    private Engine eng;

/** A global variable which tells us whether we are running as an 
Applet.
@see AppletFrame
*/
    public static boolean inapplet = true;

    public WPrologGUI(Frame f) {
	// super();
	parent = f;
    }

    public void init() {
	initGUI();
    }
     
    public void initGUI() {

	int offset = 0; // used later to handle nofilefield

	GridBagConstraints c = new GridBagConstraints();

	GridBagLayout gridbag = new GridBagLayout();

        setLayout(gridbag);
	String font = "Dialog";

	int fontsize = 12;

        setFont(new Font(font, Font.PLAIN, fontsize));

	// Add top item - query label
	c.gridx = 0; c.gridy = 0;
	c.gridwidth = 1; 
	c.gridheight = 1;
	c.fill = GridBagConstraints.BOTH;

	Label query = new Label("Query:");
	gridbag.setConstraints(query,c);
	add(query);

	// Add textfield and button
	c.gridy = 0; c.gridx = 1; c.gridwidth = 2;
	c.weightx = 0.1;

	querytf = new TextField();
	gridbag.setConstraints(querytf,c);
	add(querytf);

	c.gridx = 3; c.weightx = 0; 
	c.gridwidth = 1;
	// c.gridwidth = GridBagConstraints.REMAINDER;

	doquery = new Button("Run Query");
	gridbag.setConstraints(doquery,c);
	add(doquery);

	c.gridx = 4; c.weightx = 0; 
	c.gridwidth = GridBagConstraints.REMAINDER;

	more = new Button("More?");
	gridbag.setConstraints(more,c);
	add(more);

	boolean havefile = true;
	
	// Add label, textfield and two buttons ...
	c.gridx = 0; c.gridy = 1; c.gridwidth = 1;

	if (inapplet) {
		benchmark = new Button("Benchmark");
		gridbag.setConstraints(benchmark,c);
		add(benchmark);
	} else {
		Label l = new Label("File: ");
		gridbag.setConstraints(l,c);
		add(l);
	}

	c.gridy = 1;
	c.gridx = 1; c.weightx = 0.1; c.gridwidth = 2;
	if (inapplet) {
		about = new Button("About...");
		gridbag.setConstraints(about,c);
		add(about);
	} else {
		filename = new TextField();
		gridbag.setConstraints(filename,c);
		add(filename);
	}

	// Since applics use a filedialog now ...
	if (!inapplet) filename.setEditable(false);

	c.gridx = 3; c.weightx = 0; c.gridwidth = 1;

	if (inapplet) {
		prog1 = new Button("Append");
		gridbag.setConstraints(prog1,c);
		add(prog1);
	} else {
		load = new Button("Load");
		gridbag.setConstraints(load,c);
		add(load);
	}

	c.gridx = 4; c.weightx = 0; c.gridwidth = 1;
	c.gridwidth = GridBagConstraints.REMAINDER;

	if (inapplet) {
		prog2 = new Button("Documentation");
		gridbag.setConstraints(prog2,c);
		add(prog2);
	} else {
		save = new Button("Save");
		gridbag.setConstraints(save,c);
		add(save);
	}

	// if (inapplet) save.disable();

	// Add label
	c.gridy = 2 + offset;
	c.gridx = 0;
	c.gridwidth = GridBagConstraints.REMAINDER;
	c.weightx = 0; c.weighty = 0;

	Label l2 = new Label("Program: ");
	gridbag.setConstraints(l2,c);
	add(l2);

	// Add TextArea 
	c.gridy = 3 + offset;
	c.gridx = 0;
	c.gridwidth = GridBagConstraints.REMAINDER;
	c.weightx = 0.5; c.weighty = 0.5;

	program = new TextArea();
	gridbag.setConstraints(program,c);
	add(program);

	// Add label, two toggles and two buttons
	c.gridy = 4 + offset; 
	c.gridx = 0;
	c.gridwidth = 1;
	c.weightx = 0.03; c.weighty = 0;

	Label l3 = new Label("Results: ");
	gridbag.setConstraints(l3,c);
	add(l3);

	c.gridx = 1; c.weightx = 0.7;
	Label ldummy = new Label(" ");
	gridbag.setConstraints(ldummy,c);
	add(ldummy);

	// c.gridwidth = 1;
	c.gridx = 2; c.weightx = 0;
	occurcheck = new Checkbox("Occur Check");
	gridbag.setConstraints(occurcheck,c);
	add(occurcheck);

	c.gridx = 3; c.weightx = 0;
	trace = new Checkbox("Trace");
	gridbag.setConstraints(trace,c);
	add(trace);

	c.weightx = 0;
	c.gridx = 4; c.gridwidth = GridBagConstraints.REMAINDER;
	clear = new Button("Clear");
	gridbag.setConstraints(clear,c);
	add(clear);


	// Add TextArea 
	c.gridy = 5 + offset;
	c.gridx = 0;
	c.gridwidth = GridBagConstraints.REMAINDER;
	c.weightx = 0.5; c.weighty = 0.5;

	results = new TextArea();
	results.setEditable(false);
	gridbag.setConstraints(results, c);
	add(results);

	validate();
    }

/** start doesn't do anything */
    public void start() { }

    public boolean action(Event e, Object arg) {
	if (e.target == doquery) {
		Term t; String s;

		s = querytf.getText();

		try {
			t = new Term(new ParseString(s,querytf));
		} catch (Exception f) {
			results.appendText("Can't parse query!\n");
			eng = null;
			t = null; // dummy
		}
		if (t != null) { // parse query succeeded
		    try {
			eng = new Engine(t,
				ParseString.consult(
					program.getText(),
					new Hashtable(),
					program));
			eng.start();
		    } catch (Exception f) { // can't parse program ...
			    eng = null;
			    t = null; // dummy
		    }
		}
	}
	if (e.target == load) { 
	   if (inapplet) {
		try {
			program.setText( (String) 
			   (new URL(filename.getText())).getContent());
                } catch (MalformedURLException zzz) {
                        IO.error("WPrologGUI.action","Malformed URL: " + zzz);
                } catch (IOException zzz) {
                        IO.error("WPrologGUI.action","IO: " + zzz);
                }
	   } else {
	   // Load from application ... use file requester
		try {
			FileDialog fd = new FileDialog(parent,"W-Prolog 1.2.1");
			fd.show();

			if (fd.getFile() != null) {
				filename.setText(fd.getDirectory() 
							+ fd.getFile());
				FileInputStream fis = new FileInputStream(new 
						      File(filename.getText()));

				try { 
					byte b[] = new byte[fis.available()];
					fis.read(b); 
					fis.close();
					program.setText(new String(b,0));
				} catch (java.io.IOException zzzzz) {
					IO.error("Load","Can't read: "
						+filename.getText());
				}
			}

		} catch (FileNotFoundException zzzz) {
			IO.error("Load","Can't open: "+filename.getText());
		}
			
	   }
	}
	if (e.target == save) {
		try {
			FileOutputStream fos = new FileOutputStream(
						filename.getText());

			int len = (program.getText()).length();

			char bc[] = new char[len];
			bc = (program.getText()).toCharArray();
			byte b[] = new byte[len];
			int i;
			for (i=0;i<len;i++) b[i] = (byte) bc[i];
			
			fos.write(b);

		} catch (java.io.IOException zzzzzz) {
			IO.error("Save","Can't open: " + filename.getText());
		}
	}
	if (e.target == more)   
		if (eng != null) eng.more();
	if (e.target == clear) 
		results.setText("");
	if (e.target == occurcheck) 
		Term.occurcheck = occurcheck.getState();
	if (e.target == trace) 
		Engine.trace = trace.getState();
	if (e.target == benchmark) {
		IO.result("496 / time = LIPS");
		Engine eng;
		Term t; String s = "nrev30";

		try {
			t = new Term(new ParseString(s,null));
		} catch (Exception f) {
			WPrologGUI.results.appendText(
				"Can't parse query!\n");
			eng = null;
			t = null; // dummy
		}
		if (t != null) { // parse query succeeded
		    try {
			eng = new Engine(t,
				ParseString.consult(
"append([],X,X). append([X|Xs],Y,[X|Z]) :- append(Xs,Y,Z). nrev([],[]).  nrev([X|Xs],Zs) :- nrev(Xs,Ys) , append(Ys,[X],Zs). nrev30 :- nrev([1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0],X).",
					new Hashtable(), null));
			eng.start();
		    } catch (Exception f) {
		    // can't parse program ...
			    eng = null;
			    t = null; // dummy
		    }
		}
	}
	if (e.target == about) {
		program.setText(
	"W-Prolog 1.2.1 19 August 1999\n\n" +
	"Author: Michael Winikoff (winikoff@acm.org)\n" +
	"\nAvailable from: http://members.xoom.com/winikoff/wp\n");
	}

	if (e.target == prog1) { // append ...
		program.setText(
	"% append two lists ...\n\n" +
	"append([],X,X).\n\nappend([X|Xs],Y,[X|Z]) :- append(Xs,Y,Z).\n");
	}

	if (e.target == prog2) { 
		try {
			URL wpdocs = new URL(
				"http://members.xoom.com/winikoff/wp");
			
			WProlog.ac.showDocument(wpdocs,"_blank");

		} catch (MalformedURLException z) {}
			
	}

	
	
        return true;
    }

    public String getAppletInfo() {
        return "W-Prolog 1.2.1\n\nA Prolog interpreter written in Java\n\n" +
		"Author: Michael Winikoff\nDate: October 1996\n" +
		"Last revised: August 1999";
    }

}



/** This is only used to implement <b>once</b>. */
final class OnceMark {
	public OnceMark() {}
}

/** This is a simple record. A choicepoint contains the continuation
(a goal list) and the next clause to be tried. */
final class ChoicePoint  {
	int clausenum;
	TermList goal;

	public ChoicePoint(int cn,TermList g) {
		clausenum = cn; goal = g;
	}

	public final String toString() {
		return ("<<" + clausenum + " : " + goal + ">>");
	}
}

/** An engine is what executes prolog queries.
* @author Michael Winikoff
*/
final class Engine extends Thread {
/** The stack holds choicepoints and a list of variables
which need to be un-bound upon backtracking.
*/
	private Stack stack; 
/** We use a hashtable to store the program */
private Hashtable db; 
private TermList goal;
	private Term call;
/** This governs whether tracing is done */
	public static boolean trace = false;
/** Used to time how long queries take */
	private long time;

/** A bookmark to the fail predicate */
	private TermList failgoal;

/** 
* @param prog An initial program - this will be extended
* @param g The query to be executed
*/
public Engine(Term g, Hashtable prog) {
		Term t,t2;
		TermList tl;

		stack = new Stack(); 
		goal = new TermList(g,null);
		call = g;

		// enable underscore as first character of term
		Term.internalparse = true; 


		try {
		db = ParseString.consult(
		   ("eq(X,X). fail :- eq(c,d). print(X) :- _print(X)." +
		    "if(X,Y,Z) :- once(wprologtest(X,R)) , wprologcase(R,Y,Z)."+
		    "wprologtest(X,yes) :- call(X). wprologtest(X,no). " +
		    "wprologcase(yes,X,Y) :- call(X). " +
		    "wprologcase(no,X,Y) :- call(Y)." +
		    "not(X) :- if(X,fail,true). or(X,Y) :- call(X). "+
		    "or(X,Y) :- call(Y)." +
		    "true. call(X) :- _call(X). " +
		    "nl :- _nl. once(X) :- _onceenter , call(X) , _onceleave.")
			,prog,null);
		ParseString.resolve(db);
		} catch (Exception e) {
		  IO.fatalerror("Engine.init","Can't parse default program!");
		}

		Term.internalparse = false; 

		goal.resolve(db);

		failgoal = new TermList(new Term("fail",0), null);
		failgoal.resolve(db);

		setPriority(2);

		time = System.currentTimeMillis();

	}

final public void dump(int clausenum) {
	if (trace) {
	IO.trace("Goal: " + goal + " clausenum = " + clausenum);
	}
}

final public void run() { 
	String r;
	r = run(false); 
	IO.result(r);
}

/** run does the actual work. */
final public String run(boolean embed) { 
	boolean found;
	String func;
	int arity, clausenum;
	TermList clause, nextclause, ts1, ts2;
	Term t;
	Object o;
	ChoicePoint cp;
	Term vars[];

	clausenum = 1;

while (true) {
if (goal==null) {
	Runtime r = Runtime.getRuntime();
	r.gc();
	long totalmem = r.totalMemory();
	long currentmem = r.freeMemory();
	long percent = (100 * (totalmem - currentmem)) / totalmem;
	totalmem = (long) (((float) totalmem) / 1000000.0);
	return ("Yes: " + call +  "\n(approx. " + 
		(System.currentTimeMillis() - time) + " ms., memory used: " +
		percent + "% out of " + totalmem + " Mb)");
	// this.suspend();
	// return;
}

if (goal.term==null) {
	IO.fatalerror("Engine.run","goal.term is null!");
}

	func = goal.term.getfunctor();
	arity = goal.term.getarity(); // is this needed?
dump(clausenum);

	if (func.charAt(0) != '_') { 
	// ie the goal is not a system predicate AAA
		// if there is an alternative clause push choicepoint:
		if (goal.numclauses > clausenum)
			stack.push(new ChoicePoint(clausenum +1, goal));

		if (clausenum > goal.numclauses) {
			clause = failgoal;
			IO.diagnostic(func + "/" + arity + " undefined!");
			clause = new TermList(new Term("fail",0), goal);
			clause.resolve(db);
		} else 
			clause = goal.definer[clausenum];

		clausenum = 1; // reset ...
		// check unification ...
		vars = new Term[300]; // XXX arbitrary limit
		if (clause.term.refresh(vars).unify(goal.term,stack)) {
			clause = clause.next;

			// refresh clause -- need to also copy definer 
			if (clause != null) {
				ts1 = new TermList(clause.term.refresh(vars), 
						   null,clause);
				ts2 = ts1;
				clause = clause.next;

			while (clause != null) {
				ts1.next = new TermList(
						clause.term.refresh(vars),
						null,clause);
				ts1 = ts1.next;
				clause = clause.next;
			}

			// splice together refreshed clause and other goals
			ts1.next = goal.next;
			goal = ts2;

			// For GC purposes drop references to data that is not 
			// needed 
			 t = null; ts1 = null; ts2 = null; vars = null; 
			 clause = null; nextclause = null; func = null;

			} else { // matching against fact ...
			goal = goal.next;
			}
		} else { // unify failed - backtrack ...

			goal = goal.next;
			found = false;
			while (! stack.empty()) {
				o = stack.pop();

				if (o instanceof Term) {
					t = (Term) o;
					t.unbind();
				} else if (o instanceof ChoicePoint) {
					cp = (ChoicePoint) o;
					goal = cp.goal;
					clausenum = cp.clausenum;
					found = true;
					break;
				} // else if integer .. iterative deepening 
				// not implemented yet
			    }

			    // stack is empty and we have not found a choice 
			    // point ... fail ...

			    if (!found) {
				    return "No."; // IO.result("No.");
				    // this.suspend();
				    // return;
			    }
		}


	} // AAA
	else if (func.equals("_print") & arity==1) {
		if (embed) 
			System.out.print(goal.term.getarg(0).toString());
		else 
			IO.prologprint(goal.term.getarg(0).toString());
		goal = goal.next;
	}
	else if (func.equals("_nl") & arity==0) {
		if (embed) 
			System.out.println("");
		else 
			IO.prologprint("\n");
		goal = goal.next;
	}
	else if (func.equals("_call") & arity==1) {
	// System.out.println(goal.term);
	// System.out.println(goal.term.getarg(0));
		TermList templist = new TermList(goal.term.getarg(0),null);
		templist.resolve(db);
		templist.next = goal.next;
		goal = templist;
	}
	// The next two together implement once/1
	else if (func.equals("_onceenter") & arity==0) {
		stack.push(new OnceMark());
		goal = goal.next;
	}
	else if (func.equals("_onceleave") & arity==0) {
		// find mark, remove mark and all choicepoints above it.
		Stack tempstack = new Stack();
		o = stack.pop();
		while (! (o instanceof OnceMark))  {
			// forget choicepoints
			if (! (o instanceof ChoicePoint)) tempstack.push(o);
			o = stack.pop();
		}

		while (! (tempstack.empty())) 
			stack.push(tempstack.pop());

		tempstack = null;
		goal = goal.next;
	}
	else {
	IO.diagnostic("Unknown builtin: " + func + "/" + arity);
	goal = goal.next;
	}
	
} // while
} // run

/** Used from the GUI when the user hits <em>more</em>. All it does is add 
<tt>fail</tt> to the goal and lets the engine do the rest. */
final public void more() {
	goal = new TermList(new Term("fail",0), goal);
	goal.resolve(db);
	time = System.currentTimeMillis();
	this.run();
	// run();
}

} // Engine

		
/** The class IO simply centralises all IO for easy modification */
class IO {

public final static void error(String caller,String mesg) {
	WPrologGUI.results.appendText(
		"ERROR: in " + caller + " : " + mesg + "\n");
}

// fatal error ...
public final static void fatalerror(String caller,String mesg) {
	WPrologGUI.results.appendText(
		"FATAL ERROR: in " + caller + " : " + mesg + "\n");
	System.exit(1);
}

public final static void result(String s) {
	WPrologGUI.results.appendText(s+"\n");
}

public final static void diagnostic(String s) {
	WPrologGUI.results.appendText("*** "+s+"\n");
}

public final static void trace(String s) {
	WPrologGUI.results.appendText(s+"\n");
}

public final static void prologprint(String s) {
	WPrologGUI.results.appendText(s);
}
}



// Var is a type of Term. 
/** A Term is the basic data structure in Prolog. 
There are three types of terms:
<ol>
<li> Values (ie. have a functor and arguments)
<li> Variables (ie. unbound)
<li>  References (bound to another variable)
</ol>
*/
final class Term {
	private String functor; private int arity; private Term args[];
	private static int varnum=1;
	// If bound is false then term is a free variable
	private boolean bound; private int varid;
	// If bound is true and deref is true then the term is 
	// a reference to ``ref''
	private boolean deref; private Term ref;

/** Controls whether occurcheck is used in unification.
Note that in version 1.0 the occurcheck was always performed
which accounted for the lower performance.
*/
	static public boolean occurcheck = false;

/**
prettyprint controls printing of lists as <tt>[a,b]</tt>
rather than <tt>cons(a,cons(b,null))</tt>
*/
	static public boolean prettyprint = true;

/**
Controls whether predicates can begin with an underscore.
Beginning a system with an underscore makes in inaccessible 
to the user.
*/
	static public boolean internalparse = false;

/** create fresh var */
	public Term() { 
		varid = varnum++;
		bound = false;
		deref = false;
	}

/** create var with specified varid */
	public Term(int i) { 
		varid = i;
		bound = false;
		deref = false;
	}

/** create a term with  a given functor and arity.
@param s - the functor
@param a - the arity
*/
	public Term(String s,int a) {
		functor = s; arity = a;
		bound = true; deref = false;
		args = new Term[arity];
	}

/** Binds a variable to a term */
	public final void bind(Term t) {
		if (this==t) return; // XXXX binding to self should have no effect
		if (!bound) {
			bound = true; deref = true;
			ref = t;
		} else IO.error("Term.bind(" + this + ")" ,
			     "Can't bind nonvar!"); 
	}

/** Unbinds a term -- ie. resets it to a variable */
	public final void unbind() {
		bound = false; ref=null;
		// XXXX Now possible for a bind to have had no effect so ignore safety test
		// XXXX if (bound) bound = false;
		// XXXX else IO.error("Term.unbind","Can't unbind var!");
	}

/** Used to set specific arguments. A primitive way of 
constructing terms is to create them with Term(s,f) and then 
build up the arguments. Using the parser is much simpler */
	final public void setarg(int pos,Term val) {
		// only to be used on bound terms
		if (bound & (!deref)) args[pos] = val;
		else  IO.error("Term.setarg(" + pos + "," + val + ")",
			    "Can't setarg on variables!");
	}

/** Retrieves an argument of a term */
	public final Term getarg(int pos) {
		// should check if pos is valid
		if (bound) {
			if (deref) {return ref.getarg(pos);}
			else {return args[pos];}
		} else {
			IO.fatalerror("Term.getarg", 
				      "Error - lookup on unbound term!");
			return null; // dummy ... never reached 
		}
	}

/** Gets the functor of a term */
	public final String getfunctor() {
		if (bound) {
			if (deref) {return ref.getfunctor();}
			else return functor;
		} else return "";
	}

/** Gets the arity of a term */
	public final int getarity() {
		if (bound) {
			if (deref) {return ref.getarity();}
			else return arity;
		} else return 0;
	}

/** Checks whether a variable occurs in the term */
// XXXX Since a variable is not considered to occur in itself 
// XXXX added occurs1 and a new front end called occurs.
	final boolean occurs(int var) {
		if (varid==var) return false; 
		else return occurs1(var);
	}

	final boolean occurs1(int var) {
		if (bound) {
		if (deref) return ref.occurs1(var);
		else { // bound and not deref
				for (int i=0 ; i < arity ; i++) 
					if (args[i].occurs1(var)) return true;
				return false;
			}
		} else // unbound
		  return (varid==var);
	}

/** Unification is the basic primitive operation in logic programming. 
* @param s - the stack is used to store the addresses of variables which are 
bound by the unification. This is needed when backtracking.
*/
	final public boolean unify(Term t,Stack s) {
		if (bound & deref) return ref.unify(t,s);
		if (t.bound & t.deref) return unify(t.ref,s);
		if (bound & t.bound) { // bound and not deref
			if (functor.equals(t.getfunctor()) & (arity==t.getarity()))
			{
				for (int i=0; i<arity ; i++) 
					if (!args[i].unify(t.getarg(i),s)) 
						return false;
				return true;

			} 
			else  
				return false; // functor/arity don't match ...

		}  // at least one arg not bound ...
		if (bound) {
		// return t.unify(this,s);
			// XXXX Added missing occur check
			if (occurcheck) {if (this.occurs(t.varid)) return false;}  // XXXX
			t.bind(this);
			s.push(t);
			return true;
		} 
		// Do occurcheck if turned on ...
		if (occurcheck) {if (t.occurs(varid)) return false;}
		this.bind(t);
		s.push(this); // save for backtracking
		return true;
	}

/** refresh creates new variables. If the variables allready exist in 
its argument then they are used - this is used when parsing a clause so 
that variables throught the clause are shared.  Includes a copy operation */
	public final Term refresh(Term l[]) {
		Term t;
		if (bound) {
		if (deref) return ref.refresh(l);
		else { // bound & not deref
			t = new Term(functor,arity);
			// t.bound = true; t.deref = false; 
			// t.functor = functor; t.arity = arity;
			for (int i=0;i<arity;i++) 
				t.args[i] = args[i].refresh(l);
			return t;
		}} else //unbound
			return getvar(l,varid);
	}

	private final Term getvar(Term l[], int v) {
		if (l[v]==null) l[v] = new Term();
		return l[v];
	}

/** Displays a term in standard notation */
	public final String toString() {
		String s;
		if (bound) {
		  if (deref) return ref.toString();
		  else {
			if (functor.equals("null") & arity==0 & prettyprint)
				return "[]";
			if (functor.equals("cons") & arity==2 & prettyprint) {
				Term t; 
				s = "[" + args[0];
				t = args[1];

				while (t.getfunctor().equals("cons") &
					t.getarity() == 2) {
					s = s + "," + t.getarg(0);
					t = t.getarg(1);
				}

				if (t.getfunctor().equals("null") &
					t.getarity() == 0) 
					s = s + "]";
				else s = s + "|" + t + "]";

				return s;
			} else {
				s = functor;
				if (arity > 0) {
					s = s + "(";
					for (int i=0; i < (arity - 1); i++) 
						s =s + args[i].toString() + ",";
					s = s + args[arity-1].toString() + ")";
				}
			}
			return s;
		  }
		} else return ("_" + varid);
	}

/** This constructor is the simplest way to construct a term. The term is 
given in standard notation. 
Example <tt>Term(new ParseString("p(1,a(X,b))"))</tt>
@see ParseString
*/
	public Term(ParseString ps) throws Exception {
		Term ts[]; int i=0; Term t;

		ts = new Term[300]; // arbitrary 

		if (Character.isLowerCase(ps.current()) | 
			(internalparse & ps.current() == '_') ) {
			functor = ps.getname();
			bound = true; deref = false;

			if (ps.current() == '(') {
				ps.advance(); ps.skipspace();
				ts[i++] = new Term(ps);
				ps.skipspace();

				while (ps.current() == ',') {
					ps.advance(); ps.skipspace();
					ts[i++] = new Term(ps);
					ps.skipspace();
				}

				if (ps.current() != ')') 
					ps.parseerror("Expecting: ``)''");

				ps.advance();

				args = new Term[i];

				for (int j = 0 ; j < i ; j++)
					args[j] = ts[j];
				
				arity = i;

			} else arity = 0;
		} else

		if (Character.isUpperCase(ps.current())) {
			bound = true; deref = true;
			ref = ps.getvar();
		} else

		if (Character.isDigit(ps.current())) {
			functor = ps.getnum();
			arity = 0;
			bound = true ; deref = false;
		} else

		if (ps.current() =='[') {
			ps.advance();

			if (ps.current() == ']') {
				ps.advance();
				functor = "null"; arity = 0;
				bound = true; deref = false;
			} else {
				ps.skipspace();
				ts[i++] = new Term(ps);
				ps.skipspace();

				while (ps.current()==',') {
					ps.advance(); ps.skipspace();
					ts[i++] = new Term(ps);
					ps.skipspace();
				}

				if (ps.current() == '|') {
					ps.advance(); ps.skipspace();
					ts[i++] = new Term(ps);
					ps.skipspace();
				} else ts[i++] = new Term("null",0);

				if (ps.current() != ']') 
					ps.parseerror("Expecting ``]''");

				ps.advance();

				bound = true; deref = false;

				functor = "cons"; arity = 2;
				args = new Term[2];

				for (int j=i-2; j>0 ; j--) {
					t = new Term("cons",2);
					t.setarg(0,ts[j]);
					t.setarg(1,ts[j+1]);
					ts[j] = t;
				}

				args[0] = ts[0]; args[1] = ts[1];

			}
		} else ps.parseerror(
			"Term should begin with a letter, a digit or ``[''");
	}

}


/** Simple a list of terms. */
final class TermList {
        Term term;
        TermList next;

	// The next two are initialised in ParseString.consult
	public TermList definer[] = null;  // where is this atom defined?
	public int numclauses=0; 	   // How many clauses define this atom?

        public TermList(Term t, TermList n) {
                term = t; next = n;
        }

        public TermList(Term t, TermList n,TermList d) {
                term = t; next = n;
		definer = d.definer;
		numclauses = d.numclauses;
        }

	public final String toString() {
		String s; TermList tl;
		s = new String("[" + term.toString());
		tl = next;
		while (tl != null) {
			s = s + ", " + tl.term.toString();
			tl = tl.next;
		}

		if (definer != null) s = s + "( " + numclauses + " clauses)";

		return s + "]";
	}

/** Looks up which clauses define atoms once and for all */
	public void resolve(Hashtable db) {
	    if (definer == null) {
		numclauses = 0;
		while (db.get(
			term.getfunctor() + "/" + term.getarity() + "-"
			+ (1+numclauses)) != null) numclauses++;
		
		definer = new TermList[numclauses+1]; // start numbering at 1

		for (int i=1; i <= numclauses ;i++) {
			definer[i] = (TermList) db.get(
					term.getfunctor() + "/" + 
					term.getarity() + "-" + (i));
		//	IO.trace("resolve: " + term.getfunctor() + "/" +
		//			term.getarity() + "-" + i +
		//			" = " + definer[i]);
		}

		if (next != null) next.resolve(db);
	    }
	}

/** Used for parsing  clauses. */
	public TermList(ParseString ps) throws Exception {
		Term ts[]; int i=0; TermList tsl[];

		ts = new Term[300]; // arbitrary
		tsl = new TermList[300]; // arbitrary

		ts[i++] = new Term(ps); ps.skipspace();

		if (ps.current() == ':') {
			ps.advance();

			if (ps.current() != '-') 
				ps.parseerror("Expecting ``-'' after ``:''");

			ps.advance(); ps.skipspace();

			ts[i++] = new Term(ps);
			ps.skipspace();

			while (ps.current() == ',') {
				ps.advance(); ps.skipspace();
				ts[i++] = new Term(ps);
				ps.skipspace();
			}

			tsl[i] = null;

			for (int j = i - 1 ; j > 0 ; j--)
				tsl[j] = new TermList(ts[j],tsl[j+1]);
			
			term = ts[0];
			next = tsl[1];

		} else {
			term = ts[0]; next = null;
		}

		if (ps.current() != '.')
			ps.parseerror("Expecting ``.''");
			
		ps.advance();
	}
}



/** Parsing library */
final class ParseString  {
	private String str;
	private int posn, start, varnum;
/** source stores a pointer to the TextField (for the query) or the 
TextArea (for the program) so if an error occurs in parsing we can highlight
the correct part of GUI.
*/
	private TextComponent source;
/** The hashtable stores indeces for variables that have been encountered.
It isn't automatically reset since we want to have variables common 
in a clause */
	private Hashtable vardict;

	final public String toString() {
		return "{" + str.substring(0,posn) + " ^ " + 
			str.substring(posn) + " | " + vardict + " }";
	}
/** Initialise variables */
	public ParseString(String s,TextComponent tc) {
		str = s; source = tc;
		posn = 0; start = 0; varnum = 0;
		vardict = new Hashtable();
	}
/** Get the current character */
	final public char current() {
		if (this.empty()) return '#'; //  can't be space
		else return str.charAt(posn);
	}
/** Is the parsestring empty? */
	final public boolean empty() {
		return posn == (str.length());
	}
/** Move a character forward */
	final public void advance() {
		posn ++;
		if (posn >= str.length()) posn = str.length();
	}

	// all three get methods must be called before advance.
/** Recognise a name (sequence of alphanumerics) */
	final public String getname() {
		String s;

		start = posn; posn ++;
		while ( Character.isDigit(this.current()) |
			Character.isLowerCase(this.current()) |
			Character.isUpperCase(this.current())) 
			posn++;
		s = str.substring(start,posn);
		if (posn >= str.length()) posn = str.length() ; 

		return s;
	}
/** Recognise a number */
	public String getnum() {
		String s;

		start = posn; posn ++;
		while (Character.isDigit(this.current())) 
			posn++;
		s = str.substring(start,posn);
		if (posn >= str.length()) posn = str.length() ; 

		return s;
	}

/** Get the Term corresponding to a name. If the name is new, then create a 
new variable */
	final public Term getvar() {
		String s; Term t;
		s = this.getname();
		t = (Term) vardict.get(s);

		if (t==null) {
                        t = new Term(varnum++); // XXXX wrong varnum??
			vardict.put(s,t);
		}

		return t;
	}
/** Handle errors in one place */
	final public void parseerror(String s) throws Exception {
		IO.diagnostic("Unexpected character : " + s);
		source.select(posn,posn+1);
		throw new Exception();
	}

/** Skip spaces. Also skips Prolog comments */
	final public void skipspace() throws Exception {
		while (Character.isSpace(this.current()))
			this.advance();
		skipcomment();
	}

	final private void skipcomment() throws Exception {
		if (current() == '%')  {
			while (current() != '\n' & current() != '#') advance();
			skipspace();
		}
		if (current() =='/') {
			advance();
			if (current() !='*') parseerror("expecting ``*''");
			else {
				advance();
				while (current() != '*' & current() != '#')
					advance();
				advance();
				if (current() !='/') 
					parseerror("expecting ``/''");
				advance();
			}
			skipspace();
		}
	}
/** This resets the variable dictionary. */
	final public void nextclause() {
		// create new variables for next clause
		vardict = new Hashtable();
		varnum = 0;
	}
/** This takes a hashtable and extends it with the clauses in the 
string. 
@param db - an initial program, it is augmented with the clauses parsed.
@param s - a string representing a Prolog program 
*/
	final public static Hashtable consult(String s,Hashtable db,
							TextComponent tc) 
		throws Exception {
		ParseString ps = new ParseString(s,tc); 
		String func, prevfunc, index;
		int clausenum, arity, prevarity;
		TermList tls;

		ps.skipspace();
		prevfunc = ""; prevarity = -1;
		clausenum = 1;

		while (! ps.empty()) {
			tls = new TermList(ps);

			func = tls.term.getfunctor();
			arity = tls.term.getarity();

			if (func.equals(prevfunc) & arity==prevarity) 
				clausenum++;
			else {
				clausenum = 1;
				prevfunc = func;
				prevarity = arity;
			}

			index = func + "/" + arity + "-" + clausenum;

			db.put(index,tls);

			ps.skipspace();
			ps.nextclause(); // new set of vars
		}


		return db;
	} // consult

	final public static void resolve(Hashtable db) {
		// We now resolve all references.
		for (Enumeration e = db.elements() ; e.hasMoreElements() ;) {
			((TermList) e.nextElement()).resolve(db);
		}
	}
}

