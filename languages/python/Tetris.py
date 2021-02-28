"""
Tetris.py - v1.04

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

--------------------------------------------------------------------------------
TODO

AI recalculates paths too often when aispeed is low.
AI might not be selecting optimal position due to weighted pathing. For example,
if the piece is forced into a non-optimal hole, the AI guide won't show a new
locally optimal position in the hole.
Since we use weights to judge what path to follow next, we don't need to make
the initial path breadth-first. It may be simpler to parse depth first.
Frame values change to soon per level. Use Bresenham lines?
"""

from random import randrange

class Tetris(object):
	#Based off of TGM tetris. The game has 3 main states: spawing, dropping, and
	#clearing. After a piece spawns or lines are cleared, the level increases. As the
	#level increases, the frames per state decrease and the gravity increases. Pieces
	#are picked from a bag using an internal PRNG.
	#
	#If a piece is rotated so it overlaps a wall or filled cell, it will be kicked
	#left, right, down, then up to see if it can fit.
	EASY    =1<<0
	MEDIUM  =1<<1
	HARD    =1<<2
	GAMEOVER=1<<3
	SPAWNING=1<<4
	SPAWNED =1<<5
	DROPPING=1<<6
	CLEARING=1<<7
	SCANNED =1<<8
	CLEARED =1<<9
	MOVING  =1<<10

	MOVES   =8
	NOMOVE  =0
	LEFT    =1
	RIGHT   =2
	DOWN    =3
	ROTL    =4
	ROTR    =5
	SOFTDROP=6
	HARDDROP=7

	#This holds the rotated forms of the various tetris pieces. Rotations are listed
	#clockwise. For the I piece, the blocks are ordered to be reused for kick values.
	#For all pieces, the blocks are ordered to align spawning positions.
	I,O,T,L,J,S,Z=0,1,2,3,4,5,6
	PIECE=(
		#I
		( 0, 0, 1, 0, 2, 0,-1, 0),
		( 0, 0, 0, 1, 0,-1, 0,-2),
		( 0, 0, 1, 0,-1, 0,-2, 0),
		( 0, 0, 0, 1, 0, 2, 0,-1),
		#O
		( 0,-1, 1, 0, 1,-1, 0, 0),
		( 0,-1, 1, 0, 1,-1, 0, 0),
		( 0,-1, 1, 0, 1,-1, 0, 0),
		( 0,-1, 1, 0, 1,-1, 0, 0),
		#T
		( 0,-1, 0, 0, 1, 0,-1, 0),
		( 0,-1, 0, 1,-1, 0, 0, 0),
		(-1, 0, 0, 0, 0, 1, 1, 0),
		( 0, 1, 0, 0, 1, 0, 0,-1),
		#L
		(-1,-1, 0, 0, 1, 0,-1, 0),
		( 0, 0, 0, 1, 0,-1,-1, 1),
		(-1, 0, 0, 0, 1, 0, 1, 1),
		( 0, 1, 0, 0, 0,-1, 1,-1),
		#J
		( 0, 0, 1, 0, 1,-1,-1, 0),
		( 0, 0, 0, 1, 0,-1,-1,-1),
		(-1, 1,-1, 0, 0, 0, 1, 0),
		( 0, 0, 1, 1, 0,-1, 0, 1),
		#S
		( 0,-1, 0, 0, 1, 0,-1,-1),
		( 0, 0,-1, 1, 0,-1,-1, 0),
		( 0,-1, 0, 0, 1, 0,-1,-1),
		( 0, 0,-1, 1, 0,-1,-1, 0),
		#Z
		( 0,-1, 0, 0, 1,-1,-1, 0),
		(-1, 0, 0, 1,-1,-1, 0, 0),
		( 0,-1, 0, 0, 1,-1,-1, 0),
		(-1, 0, 0, 1,-1,-1, 0, 0)
	)

	def __init__(self,width=10,height=20,flags=EASY):
		assert(width>=0 and height>=0)
		#state
		self.state=flags&(Tetris.EASY|Tetris.MEDIUM|Tetris.HARD)
		self.stateframe=0
		self.frameunit=60
		self.spawnframes=0
		self.lockframes=0
		self.clearframes=0
		self.gravityden=256
		self.gravity=0
		self.level=0
		self.cleared=0
		#playing grid
		self.width=width
		self.height=height
		self.grid=[[0]*width for y in range(height)]
		self.linecount=[0]*height
		#piece randomizer
		self.bagsum=0
		self.bagcnt=[0]*7
		self.next=0
		#current piece
		self.drop=0
		self.dropx=0
		self.dropy=0
		self.droprem=0
		#AI player. The grid holds all possible piece states for the AI.
		#ailagframes is the simulated reaction time of the AI. 0=instant.
		self.ailagframes=self.frameunit//4
		self.aigrid=None
		self.aitmp=None
		self.aiheap=0
		self.ailink=None
		self.aicopy=None
		self.reset()

	def reset(self):
		#Reset to an empty grid with level=0. Set the first piece to not be S or Z.
		self.state=(self.state&(Tetris.EASY|Tetris.MEDIUM|Tetris.HARD))|Tetris.SPAWNING
		self.stateframe=0
		for y in range(self.height):
			row=self.grid[y]
			for x in range(self.width): row[x]=0
			self.linecount[y]=0
		self.level=0
		self.cleared=0
		self.levelconstants()
		#Don't let S or Z be the first spawn.
		for i in range(7): self.bagcnt[i]=0
		self.bagsum=0
		next=Tetris.S*4
		while next==Tetris.S*4 or next==Tetris.Z*4:
			next=self.gennext()
		self.next=next
		self.drop=0
		self.dropx=0
		self.dropy=0
		self.droprem=0

	def levelconstants(self):
		#Set the state frames and gravity based on the level and difficulty of the game.
		#As the level increases, scale the state frames to their minimum values and scale
		#the gravity to its maximum value. All numerators are over 256.
		num=(128,128,256,480)
		if self.state&Tetris.MEDIUM: num=(85,85,128,640)
		if self.state&Tetris.HARD  : num=(64,64,85,960)
		max,unit,level=999,self.frameunit,self.level
		level=level if level<max else max
		den,inv,self.level=256*max,max-level,level
		self.spawnframes=(unit*(128*inv+num[0]*level))//den
		self.lockframes=(unit*(128*inv+num[1]*level))//den
		self.clearframes=(unit*(256*inv+num[2]*level))//den
		self.gravity=(self.gravityden*(240*inv+num[3]*level))//(den*unit)

	def gennext(self):
		#Pick the next tetris piece from a bag of pieces. The bag has a count of how many
		#of each piece are in the bag. When a piece is picked, decrease its count.
		#bagcnt=-1=infinity.
		bagsum=self.bagsum
		bagcnt=self.bagcnt
		#If we have emptied the bag, refill it.
		if bagsum==0:
			for i in range(7):
				bagcnt[i]=3
				bagsum+=bagcnt[i]
		#Pick the next piece.
		rand=randrange(bagsum)
		next=0
		while bagcnt[next]<=rand:
			rand-=bagcnt[next]
			next+=1
		#If there's a finite number of this piece, decrement its count.
		if bagcnt[next]!=-1:
			bagcnt[next]-=1
			bagsum-=1
		self.bagsum=bagsum
		#If the grid has width 1, only the I piece can possibly spawn.
		if self.width<=1:
			next=Tetris.I
		#Format the piece for the piece array.
		return next*4

	def advance(self,frames=1):
		#Advance the state of the game by the number of frames given.
		state=self.state
		stateframe=self.stateframe
		while (state&Tetris.GAMEOVER)==0:
			if (state&Tetris.SPAWNING)!=0:
				#If this is the beginning of the spawn state, initialize a new piece.
				if (state&Tetris.SPAWNED)==0:
					self.drop=self.next
					self.next=self.gennext()
					self.droprem=0
					#If the grid is too small, the piece may need to be rotated and shifted to be
					#spawnable.
					piece=Tetris.PIECE[self.drop]
					self.drop+=self.width<=piece[4]-piece[6]
					piece=Tetris.PIECE[self.drop]
					self.dropx=(self.width-1-piece[4]-piece[6])//2
					self.dropy=self.height-1-piece[3]
					#Since we have a piece to control, allow movement. We need to set the tetris
					#state's actual state so move() knows we can move.
					state|=Tetris.SPAWNED|Tetris.MOVING
					self.state=state
					#Use a null move to test if any of the cells we are spawning on are already
					#filled. If any are filled, its game over.
					if self.canmove(Tetris.NOMOVE)==0:
						state^=Tetris.GAMEOVER^Tetris.MOVING
						break
				if stateframe>=self.spawnframes:
					#We have finished spawning, so move on to dropping.
					state^=Tetris.SPAWNING^Tetris.SPAWNED^Tetris.DROPPING
					stateframe=0
				elif frames>=1:
					#Otherwise, advance by 1 frame.
					frames-=1
					stateframe+=1
				else:
					break
			elif (state&Tetris.DROPPING)!=0:
				#Check if we can lock immediately. Specifically, if lockframes=0.
				shift=self.canmove(Tetris.DOWN)
				if shift==0 and stateframe>=self.lockframes:
					state^=Tetris.DROPPING^Tetris.CLEARING^Tetris.MOVING
					stateframe=0
					continue
				#To actually fall, we need at least 1 frame.
				if frames<1:
					break
				frames-=1
				stateframe+=1
				#If the piece can fall.
				if shift!=0:
					#Add gravity to the fractional remainder we are falling by. If we have fallen,
					#move the piece down. Also, reset stateframe to prevent the piece from locking.
					self.droprem+=self.gravity
					fall=self.droprem//self.gravityden
					self.droprem%=self.gravityden
					while fall>=1 and self.move(Tetris.DOWN)!=0: fall-=1
					stateframe=0
				#If we cannot fall, the fractional remainder we are falling by should be 0.
				self.droprem=self.droprem if self.canmove(Tetris.DOWN)!=0 else 0
			elif (state&Tetris.CLEARING)!=0:
				#Set the piece and check for cleared lines. If any lines were cleared, set the
				#cleared flag.
				width=self.width
				grid=self.grid
				linecount=self.linecount
				if (state&Tetris.SCANNED)==0:
					state|=Tetris.SCANNED
					piece=Tetris.PIECE[self.drop]
					color=self.drop//4+1
					dropx=self.dropx
					dropy=self.dropy
					for i in range(0,8,2):
						#Set the blocks of the piece.
						y=piece[i+1]+dropy
						line=grid[y]
						line[piece[i]+dropx]=color
						linecount[y]+=1
						#If we have cleared a line, clear it, but don't shift the lines above it yet.
						if linecount[y]==width:
							state|=Tetris.CLEARED
							for x in range(width): line[x]=0
				if (state&Tetris.CLEARED)==0 or stateframe>=self.clearframes:
					#If we are done pausing for the cleared lines, move to spawning.
					state^=Tetris.CLEARING^Tetris.SCANNED^Tetris.SPAWNING
					stateframe=0
					#If we have cleared lines, shift all non-empty lines down. Cleared lines are
					#marked by linecount=width.
					cleared=0
					if (state&Tetris.CLEARED)!=0:
						state^=Tetris.CLEARED
						height=self.height
						for y in range(height):
							count=linecount[y]
							if count!=width:
								dst=y-cleared
								line=grid[y]
								grid[y]=grid[dst]
								grid[dst]=line
								linecount[y]=linecount[dst]
								linecount[dst]=count
							else:
								linecount[y]=0
								cleared+=1
					#Advance the level by {0,1,2,4,6} for each line cleared, and +1 for the piece
					#that's about to spawn. Then recalculate level values.
					self.cleared+=cleared
					self.level+=(1,2,3,5,7)[cleared]
					self.levelconstants()
				elif frames>=1:
					#We are paused for the cleared lines.
					frames-=1
					stateframe+=1
				else:
					break
			else:
				break
		self.stateframe=stateframe
		self.state=state

	#--------------------------------------------------------------------------------
	#Movement
	#--------------------------------------------------------------------------------

	def canmove(self,move):
		#Test if we can move the piece by moving the piece and discarding any changes.
		return self.move(move,True)

	def move(self,move,testing=False):
		#Unified move command. Returns 1 if the move was successful, otherwise returns 0.
		#If testing is true, then only test if the move is possible.
		state=self.state
		if (state&Tetris.MOVING)==0:
			return 0
		def overlap():
			for i in range(0,8,2):
				x=piece[i+0]+dropx
				y=piece[i+1]+dropy
				if x<0 or x>=width or y<0 or y>=height or grid[y][x]!=0:
					return 1
			return 0
		width,height=self.width,self.height
		grid=self.grid
		drop=self.drop
		dropx,dropy=self.dropx,self.dropy
		piece=Tetris.PIECE[drop]
		if move<=Tetris.DOWN:
			#Test if the piece can shift in a specific direction.
			dropx+=(0,-1,1,0)[move]
			dropy+=(0,0,0,-1)[move]
			if overlap()!=0: return 0
		elif move<=Tetris.ROTR:
			#Rotate the piece.
			dir=(move==Tetris.ROTR)*2-1
			drop=(drop&0x1c)|((drop+dir)&0x3)
			piece=Tetris.PIECE[drop]
			#If we are kicking the I piece, pull the kick values from the block coordinates.
			kick=[0,0,-1,0,1,0,0,-1,0,1]
			if drop<4:
				for i in range(8):
					kick[i+2]=-piece[i]
			#Try kicking. If we can kick, set the piece's orientation.
			for i in range(0,10,2):
				dropx=self.dropx+kick[i+0]
				dropy=self.dropy+kick[i+1]
				if overlap()==0: break
				if i==8: return 0
		elif move<=Tetris.HARDDROP:
			#Drop the piece.
			dropy-=1
			while overlap()==0: dropy-=1
			dropy+=1
			if move==Tetris.SOFTDROP:
				#If it can drop, advance to the beginning of the dropping state.
				if dropy==self.dropy: return 0
				state=(state&~(Tetris.SPAWNING|Tetris.SPAWNED))|Tetris.DROPPING
			else:
				#Advance to the clearing state.
				state=(state&~(Tetris.SPAWNING|Tetris.SPAWNED|Tetris.DROPPING|Tetris.MOVING))|Tetris.CLEARING
		if testing==False:
			self.drop=drop
			self.dropx=dropx
			self.dropy=dropy
			if move==Tetris.SOFTDROP or move==Tetris.HARDDROP:
				self.state=state
				self.stateframe=0
				self.droprem=0
		return 1

	#--------------------------------------------------------------------------------
	#AI
	#--------------------------------------------------------------------------------
	#An AI player to suggest the next move given a valid board state.
	#
	#We use fitness instead of entropy to grade the grid. The next move we want may
	#not be to increase the "order" of the grid. For instance, we may want to perform
	#a 4-line clear or to build up a pattern.
	#
	#When using order-1 or higher moves, limit the initial moves as much as possible,
	#as the number of states will multiply.

	class AICell():
		def __init__(self,pos,width):
			self.drop=(pos>>1)&3
			self.dropx=(pos>>3)%width
			self.dropy=(pos>>3)//width
			self.next=None
			self.nextmove=0
			self.state=0
			self.stateframe=0
			self.droprem=0
			self.link=None
			self.sort=0

	class AILink():
		def __init__(self):
			self.link=None
			self.prev=None
			self.move=Tetris.NOMOVE

	#A binary heap to sort potential moves.
	def aiheappush(self,val):
		heap,i=self.aitmp,self.aiheap
		self.aiheap+=1
		#Heap up
		while i:
			j=(i-1)>>1
			next=heap[j]
			if next.sort<val.sort: break
			heap[i]=next
			i=j
		heap[i]=val

	def aiheappop(self):
		self.aiheap-=1
		heap,count=self.aitmp,self.aiheap
		ret,bot=heap[0],heap[count]
		#Heap down.
		i=0
		while True:
			#Find the smallest child.
			j=i*2+1
			if j+1<count and heap[j+1].sort<heap[j].sort:
				j+=1
			if j>=count or bot.sort<heap[j].sort:
				break
			#Shift and continue the heap down.
			heap[i]=heap[j]
			i=j
		heap[i]=bot
		return ret

	def aimakecopy(self):
		#Allocate the AI and determine if any changes have been made that require
		#remapping the optimal move path.
		width=self.width
		aicopy=self.aicopy
		if aicopy is None or aicopy.width!=width or aicopy.height!=self.height:
			aicopy=Tetris(width,self.height)
			aicells=width*self.height*8
			aicopy.aigrid=[Tetris.AICell(i,width) for i in range(aicells)]
			aicopy.aitmp=[None]*aicells
			aicopy.aiheap=0
			aicopy.ailink=[Tetris.AILink() for i in range(aicells*Tetris.MOVES)]
		self.aicopy=aicopy
		#Check state value changes. 1=need to remap.
		vallist=(
			("state",0),("stateframe",0),("frameunit",1),("spawnframes",1),("lockframes",1),
			("clearframes",1),("gravityden",1),("gravity",1),("level",0),("cleared",0),
			("width",1),("height",1),("bagsum",0),("next",1),("drop",0),("dropx",0),
			("dropy",0),("droprem",0),("ailagframes",1)
		)
		remap=0
		if (aicopy.drop^self.drop)&0x1c: remap=1
		aidict,sdict=aicopy.__dict__,self.__dict__
		for pair in vallist:
			name,rem=pair
			if aidict[name]!=sdict[name]:
				aidict[name]=sdict[name]
				remap|=rem
		#Check grid changes.
		aicount,scount=aicopy.linecount,self.linecount
		for y in range(self.height):
			cnt=scount[y]
			if aicount[y]==cnt and (cnt==0 or cnt==width): continue
			aicount[y]=cnt
			airow,srow=aicopy.grid[y],self.grid[y]
			for x in range(width):
				if airow[x]!=srow[x]:
					airow[x]=srow[x]
					remap=1
		for i in range(7):
			aicopy.bagcnt[i]=self.bagcnt[i]
		return remap

	def aimapmoves(self,remap=False):
		#Given a valid tetris state, find all possible moves. Will used a cached copy of
		#the grid unless changes are detected or remap is true.
		#
		#We differentiate every potential state of the current piece by x/y coordinates,
		#rotation, and if it is locked. Given the original state as a seed, we determine
		#the next state by shifting, rotating, and dropping the piece and letting the
		#state advance aiframes number of times. If this next state has not been arrived
		#at yet, add it to the list of potential states, marking the current state as its
		#prior state, and continue to the next state in the list. If a state is locked,
		#skip it, since we cannot move it. Continue in this way until all potential
		#states have been listed.
		#
		#Determine if we can use the current cached moves or if we need to remap them.
		remap|=self.aimakecopy()
		aicopy=self.aicopy
		aigrid,width=aicopy.aigrid,aicopy.width
		pos=((aicopy.dropy*width+aicopy.dropx)<<3)+((aicopy.drop&3)<<1)+((aicopy.state&Tetris.MOVING)==0)
		startcell=aigrid[pos]
		if remap==0 and startcell.state!=0:
			return startcell
		aicells=width*aicopy.height*8
		#Mark all of the states as unused.
		drop=(aicopy.drop^aigrid[0].drop)&0x1c
		for i in range(aicells):
			cell=aigrid[i]
			cell.state=0
			cell.next=None
			cell.link=None
			cell.drop^=drop
		#Add the original state to the list of potential states.
		startcell.state=aicopy.state
		startcell.stateframe=aicopy.stateframe
		startcell.droprem=aicopy.droprem
		ailink,ailinkpos=aicopy.ailink,0
		aitmppos,aitmplen=0,1
		aitmp=aicopy.aitmp
		aitmp[0]=startcell
		#Process all potential states.
		while aitmppos<aitmplen:
			cell=aitmp[aitmppos]
			aitmppos+=1
			if (cell.state&Tetris.MOVING)==0:
				continue
			#Loop over all allowed moves.
			for move in range(Tetris.MOVES):
				#Reset the tetris and piece state to the currently processing state, and shift,
				#rotate, or drop the piece.
				aicopy.state=cell.state
				aicopy.stateframe=cell.stateframe
				aicopy.drop=cell.drop
				aicopy.dropx=cell.dropx
				aicopy.dropy=cell.dropy
				aicopy.droprem=cell.droprem
				if aicopy.move(move)==0:
					continue
				#Because the AI has to wait in between movements, simulate how the main
				#loop will modify the tetris state and piece state while the AI is waiting.
				state=aicopy.state
				stateframe=aicopy.stateframe
				droprem=aicopy.droprem
				frames=aicopy.ailagframes
				while True:
					if (state&Tetris.SPAWNING)!=0:
						if stateframe>=aicopy.spawnframes:
							state^=Tetris.SPAWNING^Tetris.SPAWNED^Tetris.DROPPING
							stateframe=0
						elif frames>=1:
							frames-=1
							stateframe+=1
						else:
							break
					elif (state&Tetris.DROPPING)!=0:
						shift=aicopy.canmove(Tetris.NOMOVE)
						if shift==0 and stateframe>=aicopy.lockframes:
							state^=Tetris.DROPPING^Tetris.CLEARING^Tetris.MOVING
							stateframe=0
							break
						if frames<1:
							break
						frames-=1
						stateframe+=1
						if shift!=0:
							droprem+=aicopy.gravity
							fall=droprem//aicopy.gravityden
							droprem%=aicopy.gravityden
							while fall>=1 and aicopy.move(Tetris.DOWN)!=0: fall-=1
							stateframe=0
						droprem=droprem if aicopy.canmove(Tetris.DOWN)!=0 else 0
					else:
						break
				#Quantify the new state. If it is unused, add it as a potential state.
				pos=((aicopy.dropy*width+aicopy.dropx)<<3)+((aicopy.drop&3)<<1)+((state&Tetris.MOVING)==0)
				next=aigrid[pos]
				link=ailink[ailinkpos]
				ailinkpos+=1
				link.link=next.link
				next.link=link
				link.prev=cell
				link.move=move
				if next.state==0:
					aitmp[aitmplen]=next
					aitmplen+=1
					next.state=state
					next.stateframe=stateframe
					next.droprem=droprem
		#Sort all locked positions by their fitness.
		heappush,heappop=aicopy.aiheappush,aicopy.aiheappop
		for i in range(aitmplen):
			cell=aitmp[i]
			if (cell.state&Tetris.MOVING)==0:
				aicopy.drop,aicopy.dropx,aicopy.dropy=cell.drop,cell.dropx,cell.dropy
				cell.sort=aicopy.aifitness()
				heappush(cell)
		fit=aicells
		while aicopy.aiheap:
			fit-=1
			aitmp[fit]=heappop()
		#Map duplicate floating point fitness values to ordinal values. Processing all
		#positions at once prevents a particular orientation from overwriting equivalent
		#orientations. Processing weight is determined by
		#next.sort=(cell.level*maxdist+cell.dist+1)*height-next.dropy
		height=aicopy.height
		maxdist=(width+height)*2
		sort=float("inf")
		level=0
		for f in range(fit,aicells):
			cell=aitmp[f]
			if sort-cell.sort>1e-6:
				sort=cell.sort
				level+=1
			cell.sort=(level*maxdist)*height-cell.dropy
			heappush(cell)
		#Process positions by their estimated sorting value.
		while aicopy.aiheap:
			cell=heappop()
			sort=cell.sort+cell.dropy+height
			#Process all predecessors. Add them to the heap if they haven't been added yet.
			link=cell.link
			while link is not None:
				prev=link.prev
				if prev.next is None:
					prev.sort=sort-prev.dropy
					prev.next=cell
					prev.nextmove=link.move
					heappush(prev)
				link=link.link
		return startcell

	def aifitness(self):
		#Determine the fitness of the grid including the current piece. The higher the
		#fitness, the better the state. Metrics are scaled so they are in units of
		#height. The metrics we use are:
		#
		#sumholes: The count of all holes on the grid. A cell is a hole is it is empty
		#and a filled cell is above it. Needs to be scaled by width.
		#
		#sumheight: The sum of the column heights. Needs to be scaled by width.
		#
		#rowflip: The number of times neighboring cells flip between empty and filled
		#along a row. Needs to be scaled by width.
		#
		#colflip: The number of times neighboring cells flip between empty and filled
		#along a column. Needs to be scaled by width.
		#
		#pieceheight: The height of the top block of the most recently placed piece. Do
		#not take into account line clears. Do no scale.
		#
		#sumwell2: The sum of squared well heights. A well is an opening 1 cell wide,
		#which happens to function as a chokepoint. Needs to be scaled by width.
		width,height=self.width,self.height
		grid=self.grid
		linecount=self.linecount
		#First lock in the piece.
		dropx,dropy=self.dropx,self.dropy
		piece=Tetris.PIECE[self.drop]
		pieceheight=0
		for i in range(0,8,2):
			y=piece[i+1]+dropy
			grid[y][piece[i]+dropx]+=1
			linecount[y]+=1
			#Set the height the piece was placed at.
			pieceheight=pieceheight if pieceheight>y else y
		pieceheight+=1
		#We can limit ourselves to only rows with filled cells, so find the highest
		#position with a filled cell.
		cleared=0
		ymax=0
		for y in range(height):
			if linecount[y]==width:
				cleared+=1
			elif linecount[y]!=0:
				ymax=y+1
		#Find the stats of each column.
		#Since the left and right walls are considered filled cells, any empty lines will
		#have a row flip when the left-most and right-most cells are compared against
		#their respective walls.
		sumholes=0
		sumheight=0
		rowflip=(height-ymax)*2
		colflip=0
		sumwell2=0
		for x in range(width):
			colheight=0
			wellheight=0
			covered=0
			#When determining column flips, we compare the current row with the row above it.
			#If the grid is filled, but a line is going to be cleared, we know that the top
			#row should be 0 instead of whatever is there currently.
			topcell=grid[height-1][x]!=0 if cleared==0 else 0
			for y in range(ymax-1,-1,-1):
				#If the line is filled, ignore it.
				c=0
				if linecount[y]!=width:
					line=grid[y]
					c=line[x]!=0
					#If the cell is empty and there is a filled cell above, we have a hole.
					sumholes+=(c^1)&covered
					#If the cell above is different, we have a column flip. Don't directly use
					#grid[y-1].
					colflip+=c^topcell
					#If the cell to the left is different, we have a row flip. Ignore the cell when
					#x=0; it will be compared against the left wall later.
					rowflip+=c^(line[x-(x>0)]!=0)
					topcell=c
					covered|=c
					colheight+=covered
					#If the cell is empty and we are already in a well, or the left and right
					#neighboring cells are filled, we are in a well.
					if c==0 and (wellheight!=0 or ((x<=0 or line[x-1]!=0) and (x+1>=width or line[x+1]!=0))):
						wellheight+=1
				#If we have reached the bottom row or a filled cell, the well has ended. Don't
				#directly use grid[y-1] to compare, as it may be a filled line.
				if y<=0 or c!=0:
					#Weight the well by the height squared. Testing with variable weights for each
					#height revealed values that converged around the square of the height.
					sumwell2+=wellheight*wellheight
					wellheight=0
				#Compare the left-most and right-most cells with the left and right walls.
				rowflip+=(c^1) if (x==0 or x+1==width) else 0
			#The bottom row needs to be compared against the bottom wall.
			colflip+=topcell^1
			sumheight+=colheight
		#Remove the piece from the grid.
		for i in range(0,8,2):
			y=piece[i+1]+dropy
			grid[y][piece[i]+dropx]-=1
			linecount[y]-=1
		#Given weights, determine the fitness of the grid. Normalize by the absolute sum
		#of the weights and the width of the grid. This will allow the fitnesses of
		#different grids to be compared. Do not scale by height.
		w=1.0/width if width>1 else 1.0
		fitness=(
			-0.2585706097*sumholes*w
			-0.0160887591*sumheight*w
			-0.1365051577*rowflip*w
			-0.4461359486*colflip*w
			-0.0232974547*pieceheight
			-0.1194020699*sumwell2*w)
		return fitness

	def suggestmove(self):
		#Return the optimal move to make.
		cell=self.aimapmoves()
		return cell.nextmove

	def suggestposition(self):
		#Return the optimal position to place the piece.
		cell=self.aimapmoves()
		while cell.next:
			cell=cell.next
		return (cell.drop,cell.dropx,cell.dropy)

class Console(object):
	def __init__(self):
		#Initialize the console and disable the cursor, linebreak, and echo.
		import os
		os.environ.setdefault("ESCDELAY","25")
		import curses
		self.curses=curses
		scr=curses.initscr()
		self.scr=scr
		curses.noecho()
		curses.cbreak()
		scr.keypad(1)
		scr.timeout(0)
		curses.curs_set(0)
		curses.start_color()
		curses.use_default_colors()
		colorarr=[-1,14,11,5,3,12,10,9]
		for i in range(8):
			curses.init_pair(i,-1,colorarr[i])
		self.width,self.height=-1,-1

	def close(self):
		#Restore the original console state.
		curses=self.curses
		scr=self.scr
		scr.erase()
		scr.refresh()
		scr.keypad(0)
		curses.echo()
		curses.nocbreak()
		curses.endwin()

	def flip(self):
		#If the dimensions have changed, redraw the buffer. Otherwise, only draw the
		#changes.
		width,height=self.width,self.height
		scr=self.scr
		curses=self.curses
		newheight,newwidth=scr.getmaxyx()
		if width!=newwidth or height!=newheight:
			width,height=newwidth,newheight
			cells=width*height
			self.width,self.height=width,height
			self.back=[[" ",0] for i in range(cells)]
			self.front=[[" ",0] for i in range(cells)]
			scr.erase()
			return
		back,front=self.back,self.front
		cells=width*height
		try:
			for i in range(cells):
				fcell,bcell=front[i],back[i]
				if fcell==bcell: continue
				x,y=i%width,i//width
				col=curses.color_pair(bcell[1])
				scr.addstr(y,x,bcell[0],col)
				fcell[0],fcell[1]=bcell[0],bcell[1]
		except:
			pass
		for cell in back:
			cell[0]=" "
			cell[1]=0
		scr.refresh()

	def settext(self,x,y,msg,col=0):
		#Place text at (x,y).
		width,height=self.width,self.height
		if y<0 or y>=height: return
		i,l=0,len(msg)
		if x<0: i=-x
		if l>width-x: l=width-x
		pos=y*width+x
		back=self.back
		while i<l:
			cell=back[pos+i]
			cell[0]=msg[i]
			cell[1]=col
			i+=1

	def getkey(self):
		return self.scr.getch()

if __name__=="__main__":
	#Example of playing tetris through the console.
	import time
	con=Console()
	def tetplot(x,y,magx,magy,fill):
		fill,c=fill%8,(" ","*")[fill==8]
		for i in range(magy): con.settext(x,y+i,c*magx,fill)
	def tetdraw(piece,x,y,magx,magy,fill):
		piecearr=Tetris.PIECE[piece]
		for i in range(0,8,2):
			tetplot(x+piecearr[i+0]*magx,y-piecearr[i+1]*magy,magx,magy,fill)
	try:
		tet=Tetris(10,20,flags=Tetris.HARD)
		fps,fpstime=60.0,time.time()
		#Controls=20 chars, padding=6 chars.
		width,height=tet.width,tet.height
		paused=0
		aimode,aiwait,aispeed=0,0,5
		tet.ailagframes=aispeed
		while True:
			#If the AI is playing and loses, reset.
			if aimode==0 and (tet.state&Tetris.GAMEOVER)!=0:
				tet.reset()
			#Process user inputs.
			while True:
				key=con.getkey()
				move=Tetris.NOMOVE
				if   key<0   : break
				elif key==27 : break
				elif key==97 : move=Tetris.LEFT
				elif key==100: move=Tetris.RIGHT
				elif key==115: move=Tetris.DOWN
				elif key==32 : move=Tetris.HARDDROP
				#rotation
				elif key==106: move=Tetris.ROTL
				elif key==107: move=Tetris.ROTR
				#state modifying keys
				elif key==105: aimode=(aimode+1)%3
				elif key==114: tet.reset()
				elif key==112: paused^=1
				if aimode!=0 and paused==0:
					tet.move(move)
			if key==27: break
			aiwait+=1
			if aimode==0 and paused==0 and aiwait>=aispeed:
				aiwait=0
				move=tet.suggestmove()
				tet.move(move)
			#Advance the tetris state.
			if paused==0:
				tet.advance()
			#Find the maximum possible magx/magy values that allow the grid to fit on screen.
			textpad=20+6
			magx=(con.width-4-textpad)//width
			magy=(con.height-2)//height
			normmag=(magy*5+2)//3
			if magx<normmag:
				magy=(magx*3+4)//5
			else:
				magx=normmag
			#If no good values could be found, we cannot draw.
			if magx==0 or magy==0:
				con.settext(0,0,"not enough space")
				continue
			#Print the left and right grid borders.
			magwidth=width*magx
			magheight=height*magy
			gridx=(con.width-magwidth-4-textpad)//2+2
			gridy=(con.height-magheight-2+1)//2
			for y in range(magheight):
				con.settext(gridx-2,gridy+y,"<!")
				con.settext(gridx+magwidth,gridy+y,"!>")
			#Print the bottom of the grid.
			con.settext(gridx-1,gridy+magheight,"*"*(magwidth+2))
			con.settext(gridx,gridy+magheight+1,"\\/"*(magwidth//2))
			#Print the controls.
			controlx=gridx-2+magwidth+4+6
			con.settext(controlx,gridy+ 0,"    AI: I")
			con.settext(controlx,gridy+ 1,"  move: A/S/D")
			con.settext(controlx,gridy+ 2,"rotate: J/K")
			con.settext(controlx,gridy+ 3,"  drop: SPACE")
			con.settext(controlx,gridy+ 4," pause: P")
			con.settext(controlx,gridy+ 5," reset: R")
			con.settext(controlx,gridy+ 6,"  exit: ESCAPE")
			con.settext(controlx,gridy+ 8,"    AI: "+("takeover","guide","none")[aimode])
			con.settext(controlx,gridy+ 9," level: {0}".format(tet.level))
			con.settext(controlx,gridy+10," lines: {0}".format(tet.cleared))
			con.settext(controlx,gridy+12,"  next:")
			#Draw the preview of the next piece.
			textx=gridx-2+magwidth+4+4
			tmagy=magy if magy<2 else 2
			tmagx=(tmagy*5+2)//3
			x,y=textx+7+tmagx,gridy+14
			tetdraw(tet.next,x,y,tmagx,tmagy,tet.next//4+1)
			#Draw the grid.
			for y in range(height):
				row=tet.grid[y]
				for x in range(width):
					tetplot(gridx+x*magx,gridy+(height-1-y)*magy,magx,magy,row[x])
			#Show the piece's optimal position.
			if aimode==1 and (tet.state&Tetris.MOVING)!=0:
				drop,dropx,dropy=tet.suggestposition()
				x=gridx+dropx*magx
				y=gridy+(height-1-dropy)*magy
				tetdraw(drop,x,y,magx,magy,8)
			#Draw the dropping piece.
			if (tet.state&Tetris.MOVING)!=0:
				x=gridx+tet.dropx*magx
				y=gridy+(height-1-tet.dropy)*magy
				tetdraw(tet.drop,x,y,magx,magy,tet.drop//4+1)
			#If the state has changed, update it.
			if (tet.state&Tetris.GAMEOVER)!=0 or paused:
				out="         "
				if (tet.state&Tetris.GAMEOVER)!=0: out="GAME OVER"
				if paused: out=" PAUSED "
				con.settext(gridx+magwidth//2-4,gridy+magheight//2-1,out)
			#Draw contents before we sleep.
			con.flip()
			elapsed=time.time()-fpstime
			dif=1.0/fps-elapsed
			if dif<0.0: dif=0.0
			if dif>1.0: dif=1.0
			time.sleep(dif)
			fpstime=time.time()
	finally:
		con.close()

