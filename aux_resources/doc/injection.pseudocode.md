
**Main Procedure (void)**
```
foreach ( Circuit 'c' in circuits ){	
	'c'.Analyze and inject();
}
```

**Analyze and inject (Statement 's')**
```
if ( 's' is perceptible of being injected){
	if ( 's' has not being already injected ){
		foreach (Node 'n' in 's') {
			if( ('p' = 's'.parent) not an (inline) subcircuit ){
				's'.InjectNode('n');
			} else{
				Search the primitive statements (group 'Y') which are instances of
					a) 'p' //the parent (inline) subcircuit
					b) (inline) models of 'p' // models of the parent (inline) subcircuit.
				;
				foreach( Statement 'r' in 'Y' ){
					Create a copy ('c2') of the golden circuit containing 'r';
					Create a copy ('p2') of 'p';
					Create a copy ('r2') of 'r', being instance of 'p2';
					Inject node 'n' in s' in 'p2';
					Substitute 'r' with 'r2' 
				}
			}
		}
	}
}
foreach( Statement child in s.children ){
	child.Analyze and inject();
}
```

**Inject nodes**
```
foreach ( Node n in Statement s ){	
	if ( perceptible of being injected and not already injected ){
		inject();
	}
	mark s as injected;
}
```
