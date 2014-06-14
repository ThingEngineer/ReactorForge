//-----------------------------------------------------------------------
// File:		ReactorGraph.pde
// Name:		Josh Campbell
// Date:		01/2013
// URL:			https://github.com/joshcam/ReactorForge/tree/master/software/processing/ReactorGraph
//
// Description:	Display, plotting and logging of serial data
//
// Usage:		Use rg_config.json to configure settings and data fields.
//-----------------------------------------------------------------------

import processing.serial.*;

// Globals
int g_win_w              = 1600;            // Window Width
int g_win_h              = 800;             // Window Height
JSONObject rg_config;
JSONObject core;
JSONArray fields;
JSONObject field;
cDataArray[] g_data;                                      // Trace buffers
cGraph g_graph           = new cGraph(0, 200, 1600, 600); // Graph location and size (x, y, w, h)
Serial g_serial;                                          // Serial class, incomming data to graph
PFont  g_font;
PrintWriter g_file;
boolean g_fileState      = false;                         // Initial dump file state
boolean refresh_flag     = true;                          // Initial refresh to draw graph components
int yLegendOffset        = 610;
int i;

void setup()
{
  // Load configuration settings
  rg_config = loadJSONObject("rg_config.json");
  core = rg_config.getJSONObject("core");
  fields = rg_config.getJSONArray("fields");
  //saveJSONObject(rg_config, "data/rg_config.json");
  
  frameRate(30);
  size(g_win_w, g_win_h);
  background(100, 100, 100);
  
  // Graph data buffers
  g_data = new cDataArray[fields.size()];
  for (i = 0; i < fields.size(); i++)
  {
    g_data[i] = new cDataArray(core.getInt("buffer_size"));    // Instantiate a cDataArray object for each chart trace
  }
  
  println(Serial.list());
  g_serial = new Serial(this, Serial.list()[core.getInt("port")], core.getInt("baudrate"), 'N', 8, 1.0);
  
  g_font = loadFont("ArialMT-20.vlw");
  textFont(g_font, 20);
  
  //Logo
  textSize(28);
  fill(255, 0, 0);    text("Reactor", 700, 630);
  fill(255);          text("Graph", 800, 630);
  
  // Legend Text
  textSize(20);
  fill(0, 0, 0);
  for (i = 0; i < fields.size(); i++)
  {
    JSONObject field = fields.getJSONObject(i);
    
    fill(field.getInt("r"),  field.getInt("g"), field.getInt("b"));
    text(field.getString("name"), 100, (yLegendOffset + ((i + 1) * 20)));
  }
  
  // Graph background
  strokeWeight(1);
  stroke(0);
  fill(0);
  g_graph.drawGraphBox();
   
  if (core.getBoolean("log_to_file"))
  {
    openFile();
  }
  
}

void openFile()
{
  if (g_fileState == false)
  {
    try  
      {  
        g_file = createWriter(core.getString("log_file"));
      }  
      catch(Exception e)  
      {  
        println("Error: Can't open file!");
      }
      
      g_fileState = true;
  }
}

void keyPressed()
{
  //println(keyCode);
  
  // If ESC is pressed save and close file then exit the program
  if (keyCode == 27)
  {
    if (core.getBoolean("log_to_file"))
    {
      g_file.flush(); // Writes the remaining data to the file
      g_file.close(); // Finishes the file
      exit(); // Stops the program
    }
  }
  
  // If s is pressed save and close file
  if (keyCode == 83)
  {
    if (core.getBoolean("log_to_file"))
    {
      g_file.flush(); // Writes the remaining data to the file
      g_file.close(); // Finishes the file
      g_fileState = false;  // Set the files state to false, means we have writen the file and will need to open and over write if we want to save more data
    }
  }
  
  // If space bar is pressed clear buffers
  if (keyCode == 32)
  {
    for (i = 0; i < fields.size(); i++)
    {
      g_data[i] = new cDataArray(core.getInt("buffer_size"));    // Instantiate a cDataArray object for each chart trace
    }
    
    // Blank out old data
    strokeWeight(1);
    fill(0);
    stroke(0);
    rect(10, 610, 80, 760);
    
    refresh_flag = true;      // Initiate a redraw to blank out graph if additive graphing is off
  }
}

void draw()
{
  // Read in all the avilable data so graphing doesn't lag behind
  while ( g_serial.available() >= (2*fields.size())+2 )
  {
    processSerialData();
  }
  
  // Refresh when new data is ready
  if (refresh_flag == true)
  {  
    refresh_flag = false;

    if (core.getBoolean("retrace_mode") == false)
    {
      // Graph background
      strokeWeight(1);
      stroke(0);
      fill(0);
      g_graph.drawGraphBox();
    }
    
    // Draw traces
    strokeWeight(1.0);        // Trace stroke weight
    for (i = 0; i < fields.size(); i++)
    {
      JSONObject field = fields.getJSONObject(i); 
      stroke(field.getInt("r"),  field.getInt("g"), field.getInt("b"));
      g_graph.drawLine(g_data[i], field.getInt("min"), field.getInt("max"));
    }
    
    // Blank out old display data
    strokeWeight(1);
    fill(0);
    stroke(0);
    rect(10, 610, 80, fields.size()*97);
    
    // Print data (Dispay, File and Console)
    textSize(20);
    String delim_string = "";
    for (int i = 0; i < fields.size(); i++)
    {
      int curSize = g_data[i].getCurSize();
      if (curSize > 0)
      {
        // Get current value for each data point
        int curVal = int(g_data[i].getVal(curSize-1));
        
        // Print to display
        JSONObject field = fields.getJSONObject(i); 
        fill(field.getInt("r"), field.getInt("g"), field.getInt("b"));
        text(curVal, 20, (yLegendOffset + ((i + 1) * 20)));
        
        // Dump data to a file if requested
        if (core.getBoolean("log_to_file"))
        {
          delim_string += curVal;
          if (i == fields.size() - 1)
          {
            try  
            {
              openFile();  // Open file if not already open
              g_file.println(delim_string);
            }
            catch(Exception e)  
            {
              println("Error: Can't open file!");
            }
          }
          else
          {
            delim_string += core.getString("field_delimiter");
          }
        }
        
        // Print data to Console
        //print("[" + i + "]=" + curVal + " ");
        //if (i == fields.size() - 1) print("\n");
      }
    }
  }
}

// This reads in one set of the data from the serial port
void processSerialData()
{
  int inByte = 0;
  int cur_match_pos = 0;
  int[] start_word = { unhex(core.getString("start_word_l")), unhex(core.getString("start_word_h")) };  // Datastream start word to syncronize chart data. (Low Byte/High Byte) FEBO
    
  while (g_serial.available() < 2); // Loop until we have enough bytes
  inByte = g_serial.read();
  
  // This while loop looks for the start word sent by the device.
  // This ensures the chart is synchronized to the data stream.
  while(cur_match_pos < 2)
  {
    if (inByte == start_word[cur_match_pos])
    {
      ++cur_match_pos;
      
      if (cur_match_pos == 2)
        break;
    
      while (g_serial.available() < 2); // Loop until we have enough bytes
      inByte = g_serial.read();
    }
    else
    {
      if (cur_match_pos == 0)
      {
        while (g_serial.available() < 2); // Loop until we have enough bytes
        inByte = g_serial.read();
      }
      else
      {
        cur_match_pos = 0;
      }
    }
  }
  
  while (g_serial.available() < 2*fields.size());  // Loop until we have one full dataset

  // Read in one full dataset
  {
    byte[] inBuf = new byte[2];
    int fieldBuf;
    byte sign;
    
    for (int i = 0; i < fields.size(); i++)
    {
      JSONObject field = fields.getJSONObject(i); 
      g_serial.readBytes(inBuf);
      
      fieldBuf = ((int)(inBuf[1]&0xFF) << 8) + ((int)(inBuf[0]&0xFF) << 0);    // Type conversion since Java doesn't support unsigned bytes
      
      if (field.getBoolean("signed") == true)
      {
        sign = (byte)((fieldBuf & 0x8000) >> 15);
        
        // If sign bit is 1 perform two's complement to decimal conversion (masked to 16 bits) and invert the sign
        if (sign == 1)
        {
          fieldBuf = -(((~fieldBuf) + 1) & 0xFFFF);
        }
      }
      
      g_data[i].addVal(fieldBuf);
    }
    
    refresh_flag = true;
  }
}

// This class helps mangage the arrays of data needed for graphing.
class cDataArray
{
  float[] m_data;
  int m_max_size;
  int m_start_index = 0;
  int m_end_index = 0;
  int m_cur_size;
  
  cDataArray(int max_size)
  {
    m_max_size = max_size;
    m_data = new float[max_size];
  }
  
  void addVal(float val)
  {
    m_data[m_end_index] = val;
    m_end_index = (m_end_index+1)%m_max_size;
    if (m_cur_size == m_max_size)
    {
      m_start_index = (m_start_index+1)%m_max_size;
    }
    else
    {
      m_cur_size++;
    }
  }
  
  float getVal(int index)
  {
    return m_data[(m_start_index+index)%m_max_size];
  }
  
  int getCurSize()
  {
    return m_cur_size;
  }
  
  int getMaxSize()
  {
    return m_max_size;
  }
}

// This class takes the data and helps graph it
class cGraph
{
  float m_g_width, m_g_height;
  float m_g_left, m_g_bottom, m_g_right, m_g_top;
  
  cGraph(float x, float y, float w, float h)
  {
    m_g_width     = w;
    m_g_height    = h;
    m_g_left      = x;
    m_g_bottom    = g_win_h - y;
    m_g_right     = x + w;
    m_g_top       = g_win_h - y - h;
  }
  
  void drawGraphBox()
  {
    stroke(0, 0, 0);
    rectMode(CORNERS);
    rect(m_g_left, m_g_bottom, m_g_right, m_g_top);
    
    // Draw vertical grid lines - X axis
    for (i = 0; i <= m_g_width; i += core.getInt("grid_v_step")) {
      stroke(50,50,50);
      line(i, m_g_height, i, 0);
    }
    // Draw horizontal grid lines - Y axis
    for (i = 0; i < m_g_height; i += core.getInt("grid_h_step")) {
      stroke(50,50,50);
      line(0, i, m_g_width, i);
    }
  }
  
  void drawLine(cDataArray data, float min_range, float max_range)
  {
    float graphMultX = m_g_width/data.getMaxSize();
    float graphMultY = m_g_height/(max_range-min_range);
    
    for(int i=0; i<data.getCurSize()-1; ++i)
    {
      float x0 = i*graphMultX+m_g_left;
      float y0 = m_g_bottom-((data.getVal(i)-min_range)*graphMultY);
      float x1 = (i+1)*graphMultX+m_g_left;
      float y1 = m_g_bottom-((data.getVal(i+1)-min_range)*graphMultY);
      line(x0, y0, x1, y1);
    }
  }
}
