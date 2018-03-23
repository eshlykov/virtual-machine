strings
  hello Hello! Which Fibonacci number are you interested in?
  answer Great choice! The number is
  goodbye Thank you!
.
labels
.
functions
  fibonacci
    strings
    .
    labels
      base
    .
    functions
    .
    commands
      pop
      move res reg1
      pop
      move res reg2
      push reg1
      equal reg2 0
      if res base
      equal reg2 1
      if res base
      push reg2
      subtract reg2 1
      push res
      pushaddr
      call fibonacci
      pop
      move res reg3
      pop
      subtract res 2
      push reg3
      push res
      pushaddr
      call fibonacci
      pop
      move res reg4
      pop
      add reg4 res
      move res reg5
      pop
      push reg5
      return
      label base
      pop
      push reg2
      return
    .
 .
commands
  str hello
  read
  push res
  pushaddr
  call fibonacci
  pop
  str answer
  print res
  str goodbye
  exit
.
