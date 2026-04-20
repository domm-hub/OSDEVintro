# GEMINI.md



## When you implement a new feature in my OS:
  - ### Thinking process
    - 1. Find at least 5 sources of the implementation and documentation of this feature
    - 2. Read Them
    - 3. You must be in planning mode (plan_mode)
    - 4. You must find sources before entering plan mode, and show me them
    - 5. If you are not sure of something, use the ask_user tool.
    - 6. Start thinking about how to implement.
    - 7. If not sure of something go to step 1 again
    - 8. If not sure of implementation, ask a new instance of AI (generalist agent) to give you it's opinion in an opinion.txt file in the root directory.

  - ### Message formatting
    - 1. Finish the message with what you did, what's next, and disantvantages
    - 2. If a tool is not working, stop generating.

  - ### What to do when? 
    ###### (For you: First person view)
    - 1. I start the message?
      - Always run ./bck.xsh. Ignore any external drive errors.
      - You must add the backup folder name and the edit
    - 2. If you find something wrong? (with my feedback in implementation, for example prompt)
      - Browse in backups (using search tools fastly) for a backup where it worked
      - If you dont find a working backup, empty the OldProject Folder, copy the root folder in it, and restore to an older commit.
  
  - ### Rules
    - 1. Please, do not ever use any destructive git commands in the root folder.
    - Use OSDEV forum for most sources
  
Thanks for reading.