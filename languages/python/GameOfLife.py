"""
GameOfLife.py - v1.01

Copyright (C) 2020 by Alec Dee - alecdee.github.io - akdee144@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

class GameOfLife(object):
	"""
	Conway's Game of Life
	Rules:
	A live cell with fewer than 2 live neighbors dies by underpopulation.
	A live cell with greater than 3 live neighbors dies by overpopulation.
	A dead cell with 3 live neighbors becomes alive by reproduction.
	All other cells stay the same.
	"""
	#Cells are added to the processing queue if their state has changed.
	#
	#Cell state queries are sped up by using coordinate hashing with a hash table of
	#linked lists.
	#
	#Cell states are stored in a single integer so we can advance their state with a
	#single table lookup. Their format is:
	#
	#     state=[3-6:count][2:in queue][1:prev][0:alive]

	class Cell(object):
		def __init__(self): pass

	def __init__(self):
		#Processing queue and coordinate hash table.
		self.queue=None
		self.deleted=None
		self.hashsize=1<<16
		self.hashmask=self.hashsize-1
		self.hashtable=[None]*self.hashsize
		#Neighbors that contribute to a cell's count.
		self.neighbors=((-1,-1),(0,-1),(1,-1),(-1,0),(1,0),(-1,1),(0,1),(1,1))
		#mnrule is used to manage whether to queue or delete the cell and set prev=alive.
		#We want the queue bit to be 0 as often as possible to avoid requeuing cells.
		#carule is the cellular automata rule.
		mnrule=[0]*72
		carule=[0]*72
		for i in range(72):
			alive=i&1
			queue=(i>>2)&1
			count=i>>3
			next=int((count|alive)==3)
			mnrule[i]=(i&~6)|(alive<<1)|((queue&(alive^next))<<2)
			carule[i]=(i&~3)|(alive<<1)|next
		self.mnrule=tuple(mnrule)
		self.carule=tuple(carule)

	def __iter__(self):
		"""Iterate through the coordinate tuples of all living cells."""
		for cell in self.hashtable:
			while cell:
				if cell.state&1: yield (cell.x,cell.y)
				cell=cell.next

	def clear(self):
		"""Clears the grid."""
		self.__init__()

	def hashcoords(self,x,y):
		#Hash the coordinates to a single integer.
		m=0xffffffff
		h=0x8fa1526f
		h=((h^x)+0xaaa6d553)&m
		h=((h>>10)|(h<<22))&m
		h^=h>>14
		h=(h*0x0956b1a5)&m
		h=((h^y)+0xaaa6d553)&m
		h=((h>>10)|(h<<22))&m
		h^=h>>14
		h=(h*0x0956b1a5)&m
		h=(h>>16)|(h<<16)
		return h&self.hashmask

	def makecell(self,x,y):
		#Return the cell at the given coordinates, or make a new one.
		hash=self.hashcoords(x,y)
		hashtable=self.hashtable
		next=hashtable[hash]
		cell=next
		while cell and (cell.x!=x or cell.y!=y):
			cell=cell.next
		if cell is None:
			#Make a new cell. Use a previously deleted one if possible.
			cell=self.deleted
			if cell: self.deleted=cell.queue
			else: cell=self.Cell()
			cell.x=x
			cell.y=y
			cell.state=0
			#Queue for state processing.
			cell.queue=None
			#Doubly linked pointers for the hash table.
			cell.hash=hash
			cell.prev=None
			cell.next=next
			if next: next.prev=cell
			hashtable[hash]=cell
		#If it's not queued, add it.
		if (cell.state&4)==0:
			cell.state|=4
			cell.queue=self.queue
			self.queue=cell
		return cell

	def advance(self,generations=1):
		"""Advance the state by a given number of generations."""
		hashtable,makecell=self.hashtable,self.makecell
		neighbors=self.neighbors
		while generations>0:
			generations-=1
			#Management loop. If a cell has been updated, update its neighbors. Also check if
			#the cell should be requeued or deleted.
			rule=self.mnrule
			cell=self.queue
			self.queue=None
			while cell:
				state=rule[cell.state]
				inc=((state|4)-cell.state)<<2
				cell.state=state
				#Update neighbors.
				if inc:
					x,y=cell.x,cell.y
					for n in neighbors:
						makecell(x+n[0],y+n[1]).state+=inc
				#Delete or requeue cell.
				qnext=cell.queue
				if state==0:
					prev,next=cell.prev,cell.next
					if prev: prev.next=next
					else: hashtable[cell.hash]=next
					if next: next.prev=prev
					cell.queue=self.deleted
					self.deleted=cell
				elif state&4:
					cell.queue=self.queue
					self.queue=cell
				cell=qnext
			#Cellular automata loop.
			rule=self.carule
			cell=self.queue
			while cell:
				cell.state=rule[cell.state]
				cell=cell.queue

	def setcell(self,coord,state):
		"""Set a cell to the given state."""
		state=int(state!=0)
		if self.getcell(coord)!=state:
			self.makecell(*coord).state^=1

	def getcell(self,coord):
		"""Get the state of given cell."""
		x,y=coord
		hash=self.hashcoords(x,y)
		cell=self.hashtable[hash]
		while cell and (cell.x!=x or cell.y!=y):
			cell=cell.next
		return cell.state&1 if cell else 0

	def setcells(self,pat,xy,trans=0,fmt=None):
		"""Draw a pattern on the grid.

		trans values: 0-3=rotate, 4=flip horizontally, 8=flip vertically.

		fmt   values: points, plaintext, lif, rle, file, and None.
		If fmt=None, the format will be guessed."""
		if fmt==None:
			#Guess what format we're using. Never guess "file".
			for fmt in ("rle","lif","plaintext","points"):
				try:
					return self.setcells(pat,xy,trans,fmt)
				except ValueError:
					pass
			raise ValueError("Unable to determine pattern type")
		if fmt=="file":
			#Read the file as one string, then guess what format it's in.
			with open(pat,"r") as f:
				return self.setcells(f.read(),xy,trans)
		points=[]
		if fmt=="points":
			#An array of living cell coordinates.
			a=d=1;b=c=0
			if trans&4: a=-1
			if trans&8: d=-1
			for i in range(trans&3):
				a,c=-c,a
				b,d=-d,b
			x,y=xy
			x+=a if a<b else b
			y+=c if c<d else d
			try:
				for p in pat:
					px=x+a*p[0]+b*p[1]
					py=y+c*p[0]+d*p[1]
					points.append((px,py))
			except (TypeError,IndexError):
				raise ValueError("Not an array of points")
			#Plot the points.
			setcell=self.setcell
			for p in points: setcell(p,1)
			return
		if isinstance(pat,str)==0:
			raise ValueError("Pattern must be a string")
		lines=pat.split("\n")
		if fmt=="plaintext":
			#A plaintext grid of cells. !=comment, .=dead, O=alive
			dy=0
			for line in lines:
				s="".join(line.split())
				if s and s[0]=="!": continue
				dx=0
				for c in s:
					if c=="O"  : points.append((dx,dy))
					elif c!=".": raise ValueError("Invalid plaintext character")
					dx+=1
				dy+=1
		elif fmt=="lif":
			#Life 1.06 file format.
			if not lines or lines[0]!="#Life 1.06":
				raise ValueError("Invalid Life 1.06 header")
			for i in range(1,len(lines)):
				if lines[i]=="": continue
				try:
					coord=tuple(map(int,lines[i].split()))
					if len(coord)!=2: raise ValueError("")
				except ValueError:
					raise ValueError("Unable to parse life 1.06 pattern")
				points.append(coord)
		elif fmt=="rle":
			#Run length encoding.
			head,data="",""
			for line in lines:
				s="".join(line.lower().split())
				if s:
					if   s[0]=="x": head+=s
					elif s[0]!="#": data+=s
			w,h=None,None
			try:
				arr=head.split(",")
				w,h=int(arr[0][2:]),int(arr[1][2:])
			except (ValueError,IndexError): pass
			if head!="x={0},y={1},rule=b3/s23".format(w,h):
				raise ValueError("Unable to parse RLE header")
			dx,dy,num=0,0,0
			for c in data:
				if c=="o" or c=="b":
					num+=(num==0)+dx
					while dx<num:
						if c=="o": points.append((dx,dy))
						dx+=1
				elif c=="$" or c=="!":
					dx=0;dy+=num+(num==0)
					if c=="!": break
				elif c.isdigit():
					num=num*10+ord(c)-48
					continue
				else:
					raise ValueError("Unrecognized character in RLE data")
				num=0
		else:
			raise ValueError("Format "+str(fmt)+" unrecognized")
		return self.setcells(points,xy,trans,"points")

	def getcells(self,xy=None,wh=None,fmt="plaintext"):
		"""Return the living cells in a given area. If xy or wh are None, return
		all cells. Allowed formats are: points, plaintext, lif, and rle."""
		if xy==None or wh==None:
			x=y=w=h=f=0
			for px,py in self:
				if f==0: x=w=px;y=h=py;f=1
				if x>px: x=px
				if w<px: w=px
				if y>py: y=py
				if h<py: h=py
			w-=x-f;h-=y-f
		else:
			x,y=xy;w,h=wh
			if w<0: x+=w;w=-w
			if h<0: y+=h;h=-h
		#Retrieve an array of points for all living cells.
		points=[]
		getcell=self.getcell
		for dy in range(h):
			for dx in range(w):
				if getcell((x+dx,y+dy)): points.append((dx,dy))
		if fmt=="points":
			#A list of points.
			return points
		elif fmt=="plaintext":
			#A plaintext grid of cells. !=comment, .=dead, O=alive
			w+=1
			ret=["."]*(w*h)
			for i in range(w-1,w*h,w): ret[i]="\n"
			for p in points: ret[p[1]*w+p[0]]="O"
			return "".join(ret)
		elif fmt=="lif":
			#Life 1.06 file format.
			ret="#Life 1.06\n"
			for p in points: ret+=" ".join(map(str,p))+"\n"
			return ret
		elif fmt=="rle":
			#Run length encoding.
			#class r notation used for access within addnum.
			class r:
				s="x = {0}, y = {1}, rule = B3/S23\n".format(w,h)
				e=len(s)
			lx,ly,cnt=0,0,0
			def addnum(num,t):
				if num>1: r.s+=str(num)
				if num>0: r.s+=t
				if len(r.s)-r.e>69: r.s+="\n";r.e=len(r.s)
			for p in points:
				dx,dy=p[0]-lx,p[1]-ly
				if dx or dy: addnum(cnt,"o");cnt=0
				if dy: dx=w-lx
				addnum(dx,"b")
				addnum(dy,"$")
				if dy: addnum(p[0],"b")
				lx,ly=p[0]+1,p[1]
				cnt+=1
			addnum(cnt,"o")
			return r.s+"!\n"
		raise ValueError("Format "+str(fmt)+" unrecognized")

	#Important patterns.
	pattern_glider="x=3,y=3,rule=B3/S23\nbob$2bo$3o!"
	pattern_ship="x=5,y=4,rule=B3/S23\no2bob$4bo$o3bo$b4o!"
	pattern_eater="x=4,y=4,rule=B3/S23\n2o2b$obob$2bob$2b2o!"
	pattern_gosper="x=36,y=9,rule=B3/S23\n"\
		"24bo11b$22bobo11b$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5b\n"\
		"o3b2o14b$2o8bo3bob2o4bobo11b$10bo5bo7bo11b$11bo3bo20b$12b2o!"

if __name__=="__main__":
	#Example usage.
	if "raw_input" in dir(__builtins__): input=raw_input
	life=GameOfLife()
	life.setcells(life.pattern_gosper,(10,4))
	life.setcells(life.pattern_eater,(52,32))
	while True:
		print(life.getcells((0,0),(90,38)))
		life.advance()
		input("press enter to advance")

