It is a basic text editor for now. It has the basic features that every text editor should have.<br>

# Basic Features
1. You can write stuff. Then save it. It uses gap buffer to store characters.<br>
2. You can move with arrow keys <br>
3. You can move word by word via CTRL and arrow keys. <br>
4. You can highlight words with SHIFT. <br>
5. You can copy and paste everything with CTRL+C and CTRL+V <br>
6. Writing more than a specific row count triggers scrolling.
7. Network structure is partially done. You can start the editor as a host or as a client.
   Clients can only listen to the host.
   Hosts can alter the file.
   Clients can use the other functionalities like saving file.

# Restrictions
1. It will work properly only if you adjust your terminal to be length of 40 lines or more. If it is less cursor loses its mind.
2. CTRL+SHIFT combo is not working for some reason going to fix it.
3. Printing logic is AWFUL. It prints 40 rows instantly every time. It is NOT optimal.
4. I think I have multiple leaks. Not sure tho.

# Future
1. Going to fix the things that are in restrictions.
2. I really need to write tests.
3. Going to add tree-sitter to make this a proper looking text editor.
4. This project at first planned as a multiple user editor. It has a long way to go.
