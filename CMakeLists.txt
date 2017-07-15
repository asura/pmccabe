cmake_minimum_required(VERSION 2.8)

project(pmccabe C)

add_executable(
  pmccabe
  cparse.c
  dmain.c
  gettoken.c
  io.c
  nmain.c
  pmccabe.c
)
add_executable(
  decomment
  decomment.c
)
