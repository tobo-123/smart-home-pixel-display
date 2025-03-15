struct symbol {uint8 color[5][3]; uint8 pixel[4][4];};

// Digits 0-9

symbol digit_0 {
  {{0, 0, 0},
   {50, 50, 50}},  
  {{0,1,1,1},
   {0,1,0,1},
   {0,1,0,1},
   {0,1,1,1}
  }
};

symbol digit_1 {
  {{0, 0, 0},
   {50, 50, 50}},    
  {{0,0,1,0},
   {0,1,1,0},
   {0,0,1,0},
   {0,0,1,0}
  }
};

symbol digit_2 {
  {{0, 0, 0},
   {50, 50, 50}},  
  {{0,1,1,0},
   {0,0,0,1},
   {0,0,1,0},
   {0,1,1,1}
  }
};

symbol digit_3 {
  {{0, 0, 0},
   {50, 50, 50}},   
  {{0,1,1,1},
   {0,0,1,1},
   {0,0,0,1},
   {0,1,1,1}
  }
};

symbol digit_4 {
  {{0, 0, 0},
   {50, 50, 50}},   
  {{0,1,0,0},
   {0,1,0,1},
   {0,1,1,1},
   {0,0,0,1}
  }
};

symbol digit_5 {
  {{0, 0, 0},
   {50, 50, 50}},   
  {{0,1,1,1},
   {0,1,0,0},
   {0,0,1,1},
   {0,1,1,1}
  }
};

symbol digit_6 {
  {{0, 0, 0},
   {50, 50, 50}},    
  {{0,1,0,0},
   {0,1,1,1},
   {0,1,0,1},
   {0,1,1,1}
  }
};

symbol digit_7 {
  {{0, 0, 0},
   {50, 50, 50}},  
  {{0,1,1,1},
   {0,0,0,1},
   {0,0,1,0},
   {0,0,1,0}
  }
};

symbol digit_8 {
  {{0, 0, 0},
   {50, 50, 50}},  
  {{0,1,1,1},
   {0,1,0,1},
   {0,1,1,1},
   {0,1,1,1}
  }
};

symbol digit_9 {
  {{0, 0, 0},
   {50, 50, 50}},    
  {{0,1,1,1},
   {0,1,0,1},
   {0,1,1,1},
   {0,0,0,1}
  }
};

// Weather symbols

symbol sun {
  {{0, 0, 0},
   {100, 50, 10}},  
  {{0,0,0,0},
   {0,1,1,0},
   {0,1,1,0},
   {0,0,0,0}
  }
};

symbol cloudy {
  {{0, 0, 0},
   {50, 50, 50}},  
  {{0,1,1,0},
   {1,1,1,1},
   {0,0,0,0},
   {0,0,0,0}
  }
};

symbol moon {
  {{0, 0, 0},
   {50, 50, 50}},  
  {{0,1,1,0},
   {1,1,0,0},
   {1,1,0,0},
   {0,1,1,0}
  }
};

symbol rain {
  {{0, 0, 0},
   {50, 50, 50},
   {0, 0, 100}},  
  {{0,1,1,0},
   {1,1,1,1},
   {0,2,0,2},
   {2,0,2,0}
  }
};

symbol snow {
  {{0, 0, 0},
   {50, 50, 50},
   {100, 100, 100}},  
  {{0,1,1,0},
   {1,1,1,1},
   {0,2,0,2},
   {2,0,2,0}
  }
};

symbol mist {
  {{0, 0, 0},
   {50, 50, 50},
   {100, 100, 100}},  
  {{0,0,0,0},
   {0,1,1,1},
   {2,2,2,0},
   {0,1,1,1}
  }
};

symbol partlysun {
  {{0, 0, 0},
   {50, 50, 50},
   {100, 50, 10}},  
  {{0,0,2,2},
   {0,1,1,2},
   {1,1,1,1},
   {0,0,0,0}
  }
};

symbol partlymoon {
  {{0, 0, 0},
   {50, 50, 50},
   {140, 140, 140}},  
  {{0,0,2,2},
   {0,1,1,2},
   {1,1,1,1},
   {0,0,0,0}
  }
};

symbol partlyrainsun {
  {{0, 0, 0},
   {50, 50, 50},
   {100, 50, 10},
   {0, 0, 100}},  
  {{0,0,2,2},
   {0,1,1,2},
   {1,1,1,1},
   {0,3,0,3}
  }
};

symbol partlyrainmoon {
  {{0, 0, 0},
   {50, 50, 50},
   {140, 140, 140},
   {0, 0, 100}},  
  {{0,0,2,2},
   {0,1,1,2},
   {1,1,1,1},
   {0,3,0,3}
  }
};

symbol thunderstorm {
  {{0, 0, 0},
   {50, 50, 50},
   {100, 50, 10},
   {0, 0, 100}},  
  {{0,1,1,0},
   {1,1,1,1},
   {3,2,0,3},
   {0,2,3,0}
  }
};