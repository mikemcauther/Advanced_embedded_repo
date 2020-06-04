#!/usr/bin/env python3

from tkinter import *
import tkinter.simpledialog
import time
from threading import Timer
from tkinter import font
import tkinter.font as tkFont

import tkinter as tk

import random
from random import shuffle

import SocketMonitor
import numpy as np
import re

__author__ = "KO-CHEN-CHI & 491415063"

class main():
    """
    A main class for application entry .
    """ 
    def __init__(self):
        """
        Construct a main instance for application entry.

        Parameters:
            none
        Returns:
            none
        """
        self.root = tk.Tk()
        self.root.title("some application")

        # Top bar
        self.top_import_font = font.Font(family='Question', size=15, weight='bold')
        self.top_frame = tk.Frame(self.root , highlightbackground="#fefcec", highlightcolor="#fefcec", highlightthickness=10)
        self.top_import_label = tk.Label(self.top_frame,bg="#fefcec",  font=self.top_import_font,anchor="w",fg ="#C59B48" , text="Important")
        self.top_import_label.pack(side=tk.TOP,fill="both", expand=True)

        self.top_explain_label = tk.Label(self.top_frame,bg="#fefcec",  anchor="w",justify=LEFT, text="Individual assessment items must be solely your own work. While students are encouraged to have high-level conversations about the problems they are \n trying to solve, you must not look at another student's code or coopy from it. The university uses sophisticated anti-collusion measures to automatically\n detect similarity between assignment submissions.")
        self.top_explain_label.pack(side=tk.TOP,fill="both", expand=True)

        #Game Button
        self.game_button = tk.Button(self.root,
                                    bg="#fefcec",
                                    command=self.create_game,
                                    #font=self.button_font,
                                    fg="#C59B48",
                                    text="Play Now" ,
                                    highlightbackground="red",
                                    highlightcolor="red",
                                    highlightthickness=10)
        
        # Left frame
        self.left_frame = tk.Frame(self.root, bg="#FFFFFF",highlightbackground="#FFFFFF", highlightcolor="#FFFFFF", highlightthickness=10)
        self._left_quick_question_manager = QuickQuestionManager(self.left_frame,self)
        self._left_quick_question_manager.GUI_generate_child_layout()
        
        # Right frame  
        self.right_frame = tk.Frame(self.root, bg="#FFFFFF",highlightbackground="#FFFFFF", highlightcolor="#FFFFFF", highlightthickness=10)
        self._right_long_question_manager = LongQuestionManager(self.right_frame,self)
        self._right_long_question_manager.GUI_generate_child_layout()

        # Whole Layout
        #self.top_frame.grid(row=0, column=0, columnspan=2, sticky="ew")
        #self.left_frame.grid(row=1, column=0, sticky="nsew")
        #self.right_frame.grid(row=1, column=1, sticky="nsew") 
        self.game_button.grid(row=2, column=0,columnspan = 2, sticky="nsew") 
        self.root.grid_rowconfigure(1, weight=1)
        
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_columnconfigure(1, weight=1)
        
        self.root.mainloop()

    def create_game(self):
        toplevel = Toplevel()
        toplevel.title('Game')
        toplevel.geometry('1500x1000')

        # Create widges in the new window
        self._game_app_window = toplevel
        self._game_app = GameApp(toplevel)

        self._game_app_window.focus_set()
        
    def callback_on_queue_before_add_student(self,name,manager):
        """
        Callback function before adding a student into manager queue to decide whether a student should be added into queue.

        Parameters:
            name(str)
            manager(AbstractQuestionManager)
        Returns:
            (Boolean) : Whether we allow this student to be added into queue
        """
        is_allow_to_add = True
        if self._left_quick_question_manager.is_student_name_presented(name) == True:
            is_allow_to_add = False
            False
    
        if self._right_long_question_manager.is_student_name_presented(name) == True:
            is_allow_to_add = False
            return False

        return is_allow_to_add
    
    def callback_on_queue_add_student(self,name,manager):
        """
        Callback function when a student has been added.

        Parameters:
            name(str)
            manager(AbstractQuestionManager)
        Returns:
            None
        """
        return
        
    def callback_on_queue_discard_student(self,name,manager):
        """
        Callback function when a student has been discarded by pushing RED button.

        Parameters:
            name(str)
            manager(AbstractQuestionManager)
        Returns:
            None
        """
        return

    def callback_on_queue_accept_student(self,name,manager):
        """
        Callback function when a student has been accepted by pushing GREEN button.

        Parameters:
            name(str)
            manager(AbstractQuestionManager)
        Returns:
            None
        """
        return

            
class AbstractQuestionManager():
    """
    A superclass for abstraction of Left and Right Frame and data manager .
    """ 
    def __init__(self,main_window,app_root):
        """
        Construct a superclass instance for general GUI features shared with both left and right frames.

        Parameters:
            main_window(tk.Frame) : Parent frame of all widgets on this GUI region
            app_root(main) : Application entry class
        Returns:
            None
        """
        self._general_bg_color_code = "#FFFFFF"
        self.root = main_window
        self.avg_no_student_text = "No students in queue"
        self.is_timer_stop = True
        self._app_root = app_root
        self._button_thick_color = ""
        self.title_font = font.Font(family='Question', size=20, weight='bold')
        self.button_font = font.Font(family='Question', size=10, weight='bold')
        self.example_label_list = []
        self.example_text_list = []

    def GUI_generate_child_layout(self):
        """
        Generate a group of child GUI widgets for one question type.

        Parameters:
            none
        Returns:
            none
        """

        # Title
        self.title_frame = tk.Frame(self.root, bg=self.get_main_color_code(), height=1000)
        self.example_frame = tk.Frame(self.root, bg=self.get_main_color_code())

        self.title_label = tk.Label(self.title_frame, bg=self.get_main_color_code(),fg = self.get_title_color_code(), font=self.title_font,text=self.get_title_text())
        self.title_label_small = tk.Label(self.title_frame, bg=self.get_main_color_code(), text=self.get_title_label_small_text())
        
        #self.example_label = tk.Label(self.example_frame, bg=self.get_general_bg_color_code() , text=self.get_example_text() , justify=LEFT)

        for i,example_text in enumerate(self.get_example_text_list()):
            if i == 0:
                self.example_label_list.append(tk.Label(self.example_frame, bg=self.get_general_bg_color_code() , text=example_text , justify=LEFT,anchor="w"))
            else:
                self.example_label_list.append(tk.Label(self.example_frame, bg=self.get_general_bg_color_code() , text=example_text ,justify=LEFT,highlightthickness=2,anchor="w"))
            
        #Student List
        self.queue_root_frame = tk.Frame(self.root,bg=self.get_general_bg_color_code())
        self._student_manage_queue = StudentQueue(self)

        #Student Button
        self.add_button = tk.Button(self.root,
                                    bg=self.get_button_bg_color_code(),
                                    command=self._student_manage_queue.add_student,
                                    font=self.button_font,
                                    fg="white",
                                    text=self.get_button_text() ,
                                    highlightbackground=self.get_button_thick_color(),
                                    #highlightbackground=self.get_button_thick_color(),
                                    #highlightcolor=self.get_button_thick_color(),
                                    highlightcolor="red",
                                    highlightthickness=10)

        
        
        
        # Avg time label
        self.avg_time_label = tk.Label(self.root, bg=self.get_general_bg_color_code(),anchor="w" ,  text=self.avg_no_student_text,justify=LEFT,highlightthickness=20,highlightbackground="#F2F2F2", highlightcolor="#F2F2F2")

        # Statistics : Top 10 Students
        self.statistics_root_frame = tk.Frame(self.root,bg=self.get_general_bg_color_code())
        self.statistics_manager = Statistics(self.statistics_root_frame)
        
        # Below Do Pack()
        # parent : title_frame
        self.title_label.pack(side=tk.TOP,fill="both", expand=True)
        self.title_label_small.pack(side=tk.TOP,fill="both", expand=True)        

        # parent : example_frame
        for i,example_label in enumerate(self.example_label_list):
            if i == 0:
                example_label.pack(side=tk.TOP,fill="both", expand=True)
            else:
                example_label.pack(side=tk.TOP,fill="both", expand=True,ipadx = 100)

        # parent : root
        self.title_frame.pack(side=tk.TOP,fill="both", expand=True)
        self.example_frame.pack(side=tk.TOP,fill="both", expand=True)
        self.statistics_root_frame.pack(side=tk.TOP,fill="both", expand=True)
        self.add_button.pack(side=tk.TOP)
        self.avg_time_label.pack(side=tk.TOP,fill="both", expand=True)
        self.queue_root_frame.pack(side=tk.TOP,fill="both", expand=True)

    def is_student_name_presented(self,name):
        """
        Decide whether a student name is presented on the GUI queue list.

        Parameters:
            name(str): The target student name
        Returns:
            (Boolean)
        """
        return self._student_manage_queue.is_student_name_presented(name)
            

        """
        Below functions are the get/set API to access GUI features.

        Parameters:
            None
        Returns:
            GUI widgets
        """
    def get_example_label_list(self):
        return self.example_label_list

    def get_example_text_list(self):
        return self.example_text_list
    
    def get_title_label_small_text(self):
        return self.title_label_small_text
    
    def get_title_color_code(self):
        return self._title_color_code
        
    def get_queue_frame(self):
        return self.queue_root_frame    
        
    def get_title_text(self):
        return self._title_text

    def get_button_text(self):
        return self._button_text
    
    def get_main_color_code(self):
        return self._color_code 

    def get_button_bg_color_code(self):
        return self._button_bg_color_code
    
    def get_example_text(self):
        return self._example_text

    def get_general_bg_color_code(self):
        return self._general_bg_color_code

    def get_button_thick_color(self):
        return self._button_thick_color
        
    def update_avg_time_label(self):
        """
        Update the GUI widget which is used to display average waiting time in current queue.

        Parameters:
            None
        Returns:
            None
        """
        self.avg_time_label.configure(text=self.GUI_waiting_time_display_text())

    def on_time_out(self):
        """
        Callbakc API to deal with the situation when a timer is up per 10 seconds.

        Parameters:
            None
        Returns:
            None
        """
        self._student_manage_queue.update_avg_waiting_time()
        self.update_avg_time_label()
        self._student_manage_queue.GUI_update()
        
        if self.is_timer_stop == True:
            return
        Timer(10, self.on_time_out, ()).start()
        
    def start_timer(self):
        """
        API to start a timer for updating GUI content related to time.

        Parameters:
            None
        Returns:
            None
        """
        if self.is_timer_stop == False:
            return
        self.is_timer_stop = False
        Timer(5, self.on_time_out, ()).start()

    def stop_timer(self):
        """
        API to stop a timer.

        Parameters:
            None
        Returns:
            None
        """
        self.is_timer_stop = True

    def GUI_waiting_time_display_text(self):
        """
        API to map a average waiting time to a meaningful words.

        Parameters:
            None
        Returns:
            (str)
        """
        stu_len = len(self._student_manage_queue.GUI_current_list());
        
        if stu_len == 0:
            return "No students in queue"
            
        avg_time = self._student_manage_queue.get_avg_waiting_time()
        if avg_time < 60:
            return "An average wait time of a few seconds for " + str(stu_len) + " students."
        if avg_time < 120:
            return "An average wait time of about a minute for " + str(stu_len) + " students."
        if avg_time < 3600:
            return "An average wait time of about " + str(avg_time//60) + " minutes for " + str(stu_len) + " students."
        if stu.get_waiting_time() < (3600*2):
            return "An average wait time of about a hour for " + str(stu_len) + " students."
        if stu.get_waiting_time() >= (3600*2):
            return "An average wait time of about " + str(avg_time//3600) + " hours for " + str(stu_len) + " students."

    def callback_on_queue_before_add_student(self,name):
        """
        Callback function before adding a student into manager queue to decide whether a student should be added into queue.

        Parameters:
            name(str)
        Returns:
            (Boolean) : Whether we allow this student to be added into queue
        """
        return self._app_root.callback_on_queue_before_add_student(name,self)
        
    def callback_on_queue_add_student(self,name):
        """
        Callback function when a student has been added.

        Parameters:
            name(str)
        Returns:
            None
        """
        self.update_avg_time_label()
        self.start_timer()
        self._app_root.callback_on_queue_add_student(name,self)
    
    def callback_on_queue_discard_student(self,name,stu_list):
        """
        Callback function when a student has been discarded by pushing RED button.

        Parameters:
            name(str)
        Returns:
            None
        """
        GUI_cur_list = [x for x in stu_list if x.get_is_removed() == False]
        
        self.update_avg_time_label()
        if len(GUI_cur_list) == 0:
            self.stop_timer()
        self._app_root.callback_on_queue_discard_student(name,self)    

    def callback_on_queue_accept_student(self,name,wait_time,stu_list):
        """
        Callback function when a student has been accepted by pushing GREEN button.

        Parameters:
            name(str)
        Returns:
            None
        """
        GUI_cur_list = [x for x in stu_list if x.get_is_removed() == False]

        # For Statistics
        self.statistics_manager.set_student_list(stu_list)
        self.statistics_manager.collect_wait_time_info(wait_time)
        self.statistics_manager.GUI_remove_layout()
        self.statistics_manager.GUI_generate_layout()
        
        self.update_avg_time_label()
        if len(GUI_cur_list) == 0:
            self.stop_timer()
        self._app_root.callback_on_queue_accept_student(name,self)     
        
class QuickQuestionManager(AbstractQuestionManager):
    """
    A subclass of AbstractQuestionManager for Left Frame .
    """ 
    def __init__(self,main_window,app_root):
        """
        Construct a group of GUI features related to a specific question type.

        Parameters:
            main_window(tk.Frame) : Parent frame of all widgets on this GUI region
            app_root(main) : Application entry class
        Returns:
            None
        """
        super().__init__(main_window,app_root)
        self._title_text = "Quick Questions"
        self._color_code = "#dbf3d6"
        self._example_text = "Some examples of quick questions:\n\
                                Syntax errors\n\
                                Interpreting error output\n\
                                Assignment/MyPyTutor interpretation\n\
                                MyPyTutor submission issues\n"
        self.example_text_list.append("Some examples of quick questions:")
        self.example_text_list.append("Syntax errors")
        self.example_text_list.append("Interpreting error output")
        self.example_text_list.append("Assignment/MyPyTutor interpretation")
        self.example_text_list.append("MyPyTutor submission issues")
        self._button_text = "Request Quick Help"
        self._button_thick_color = "#28C053"
        self._title_color_code = "#1F7B38"
        self._button_bg_color_code = "#9ADAA4"
        self.title_label_small_text = "< 2 mins with a tutor"
        
class LongQuestionManager(AbstractQuestionManager):
    """
    A subclass of AbstractQuestionManager for Right Frame .
    """
    def __init__(self,main_window,app_root):
        """
        Construct a group of GUI features related to a specific question type.

        Parameters:
            main_window(tk.Frame) : Parent frame of all widgets on this GUI region
            app_root(main) : Application entry class
        Returns:
            None
        """
        super().__init__(main_window,app_root)
        self._title_text = "Long Questions"
        self._color_code = "#D5EDF9"
        self._example_text = "Some examples of long questions:\n\
                                Open ended questions\n\
                                How to start a problem\n\
                                How to improve code\n\
                                Debugging\n\
                                Assignment help\n"
        self.example_text_list.append("Some examples of long questions:")
        self.example_text_list.append("Open ended questions")
        self.example_text_list.append("How to start a problem")
        self.example_text_list.append("How to improve code")
        self.example_text_list.append("Debugging")
        self.example_text_list.append("Assignment help")
        self._button_text = "Request Long Help"
        self._button_thick_color = "#31C1E3"
        self._title_color_code = "#186F93"
        self._button_bg_color_code = "#9CDAEB"
        self.title_label_small_text = "> 2 mins with a tutor"

class StudentQueue():
    """
    A class to manager a group of students.
    """
    def __init__(self, parent_manager):
        """
        Construct a manager class for managing the student list and related GUI.

        Parameters:
            parent_manager(AbstractQuestionManager) : Parent Manager class for this type of queue list
        Returns:
            None
        """
        self._parent_manager = parent_manager
        self._parent_frame = self._parent_manager.get_queue_frame()
        self.items = []
        self.map = {}
        self.title_student = Student(self._parent_frame,"Name",self,is_title_type=True)
        self.avg_waiting_time = 0

    def update_avg_waiting_time(self):
        """
        Update the value of average waiting time.

        Parameters:
            None
        Returns:
            None
        """
        stu_list = self.GUI_current_list()
        stu_list_len = len(self.GUI_current_list())

        if stu_list_len == 0:
            self.avg_waiting_time = 0
            return
        
        waiting_time_sum = 0
        for stu_obj in stu_list:
            waiting_time_sum = waiting_time_sum + stu_obj.get_waiting_time()

        self.avg_waiting_time = waiting_time_sum / stu_list_len

        """
        Below functions are the get/set API to access GUI features.

        Parameters:
            None
        Returns:
            GUI widgets
        """
    def get_avg_waiting_time(self):
        """
        Get the average waiting time value.

        Parameters:
            None
        Returns:
            (Decimal)
        """
        return self.avg_waiting_time
    
    def add_student(self):
        """
        Button callback when user pushs the "request question" button

        Parameters:
            None
        Returns:
            None
        """
        # Ask student name
        name = tk.simpledialog.askstring("Student Name Input", "Enter Your Name")

        is_allow_to_add = self._parent_manager.callback_on_queue_before_add_student(name)

        if is_allow_to_add == False:
            return
        
        # Check is it exists
        try:
            target_stu = self.map[name]
        except KeyError:
            target_stu = Student(self._parent_frame,name,self);
            self.map[name] = target_stu
        else:
            if target_stu.get_is_removed() == False:
                return
        
        target_stu.set_is_removed(False)
        target_stu.set_start_time(time.time())

        self.update_avg_waiting_time()
        self.GUI_update()
        self._parent_manager.callback_on_queue_add_student(name)
       

    def discard_student(self,name):
        """
        Button callback when user pushs the "Discard" button of one student

        Parameters:
            name(str) : the target student name
        Returns:
            None
        """
        try:
            target_stu = self.map[name]
        except KeyError:
            while False:
                break
        else:
            target_stu.set_is_removed(True)

        self.update_avg_waiting_time()
        self.GUI_update()

        self._parent_manager.callback_on_queue_discard_student(name,self.map.values())
        
    def accept_student(self,name):
        """
        Button callback when user pushs the "Accept" button of one student

        Parameters:
            name(str) : the target student name
        Returns:
            None
        """
        try:
            target_stu = self.map[name]
        except KeyError:
            while False:
                break
        else:
            target_stu.set_is_removed(True)

        target_stu.set_is_removed(True)
        target_stu.increase_asked_question_count()

        target_stu.update_waiting_time()
        self.update_avg_waiting_time()
        self.GUI_update()     

        self._parent_manager.callback_on_queue_accept_student(name,target_stu.get_waiting_time(),list(self.map.values()))

    def is_student_name_presented(self,name):
        """
        Check whether a student name has been displayed on the queue list

        Parameters:
            name(str) : the target student name
        Returns:
            (Boolean)
        """
        # Check is it exists
        try:
            target_stu = self.map[name]
        except KeyError:
            return False
        else:
            if target_stu.get_is_removed() == False:
                return True
            else:
                return False
        
    def GUI_waiting_time_display_map(self,stu):
        """
        Mapping a student waiting time a group of meaningful words

        Parameters:
            stu(Student) : the target student object
        Returns:
            (str)
        """
        if stu.get_waiting_time() < 60:
            return "a few seconds ago"
        if stu.get_waiting_time() < 120:
            return "a minute ago"
        if stu.get_waiting_time() < 3600:
            return str(stu.get_waiting_time()//60) + " minutes ago"
        if stu.get_waiting_time() < (3600*2):
            return "a hour ago"
        if stu.get_waiting_time() >= (3600*2):
            return str(stu.get_waiting_time()//3600) + " hours ago"
    
    def GUI_current_list(self):
        """
        Get current student list showed on this type of question queue

        Parameters:
            None
        Returns:
            (list)
        """
        return [x for x in self.map.values() if x.get_is_removed() == False]
    
    def GUI_update(self):
        """
        Get current student list showed on this type of question queue

        Parameters:
            None
        Returns:
            (list)
        """
        # Remove Title
        self.title_student.GUI_remove_item()
        
        # Remove all itmes
        for stu_obj in self.map.values() :
            if stu_obj.get_is_removed() == False:
                stu_obj.update_waiting_time()

            stu_obj.GUI_remove_item()

        if len(self.GUI_current_list()) == 0:
            return
        
        # Sorting
        sorted_items = self.sort_items(lambda x:(x.get_asked_question_count(),-x.get_waiting_time()))

        # Add Title
        self.title_student.GUI_add_item(0)
        
        # Add back
        i = 1
        for stu_obj in sorted_items:
            if stu_obj.get_is_removed():
                continue
            stu_obj.GUI_add_item(i)
            i = i + 1

    def sort_items(self, key):
        return sorted(self.map.values(), key=key)

class Student(object):
    """
    A class to represent a student who have ever entered a name.
    """
    def __init__(self,parent_frame,student_name,queue_manager,is_title_type=False):
        """
        Construct a class to store the informations related to a student.

        Parameters:
            parent_frame(tk.Frame) : The frame where this student belong to
            student_name(str) : This student displayed name
            queue_manager(StudentQueue) : A queue manager which this student belong to.
            is_title_type(Boolean) : Since this Student class also used to represent the title of a student list , we use this flag to distinguish
        Returns:
            None
        """
        self._name = student_name
        self._waiting_time = 0
        self._start_time = 0
        self._asked_question = 0
        self._parent_frame = parent_frame
        self._is_remove = False
        self._queue_manager = queue_manager
        self._is_title_type = is_title_type
        self._general_bg_color_code = self._queue_manager._parent_manager.get_general_bg_color_code()
        
    def GUI_add_item(self,row_index):
        """
        Add all GUI widgets related to this student 

        Parameters:
            row_index(int) : The row index which is used on student queue list.
        Returns:
            None
        """
        self._parent_frame.grid_rowconfigure(row_index, weight=1)
        
        # Index
        if self._is_title_type:
            self._index_lab = tk.Label(self._parent_frame, anchor="w" , text="#",bg=self._general_bg_color_code)
        else:
            self._index = row_index
            self._index_lab = tk.Label(self._parent_frame, anchor="w" , text=str(row_index),bg=self._general_bg_color_code)
        self._index_lab.grid(row=row_index, column=0, sticky="ew")
        self._parent_frame.grid_columnconfigure(0, weight=1)
        
        # Name
        if self._is_title_type:
            self._name_lab = tk.Label(self._parent_frame, anchor="w" , text="Name",bg=self._general_bg_color_code)
        else:
            self._name_lab = tk.Label(self._parent_frame, anchor="w" , text=self._name,bg=self._general_bg_color_code)
        self._name_lab.grid(row=row_index, column=1, sticky="ew")
        self._parent_frame.grid_columnconfigure(1, weight=1)

        # Question Asked
        if self._is_title_type:
            self._asked_question_lab = tk.Label(self._parent_frame, anchor="w" , text="Questions Asked",bg=self._general_bg_color_code)
        else:
            self._asked_question_lab = tk.Label(self._parent_frame, anchor="w" , text=str(self._asked_question),bg=self._general_bg_color_code)
        self._asked_question_lab.grid(row=row_index, column=2, sticky="ew")
        self._parent_frame.grid_columnconfigure(2, weight=1)

        # Waiting Time
        if self._is_title_type:
            self._waiting_time_lab = tk.Label(self._parent_frame, anchor="w" , text="Time",bg=self._general_bg_color_code)
            self._waiting_time_lab.grid(row=row_index, column=3,sticky="ew")
        else:
            self._waiting_time_lab = tk.Label(self._parent_frame, anchor="w" , text=self._queue_manager.GUI_waiting_time_display_map(self),bg=self._general_bg_color_code)
            self._waiting_time_lab.grid(row=row_index, column=3, sticky="ew")
        self._parent_frame.grid_columnconfigure(3, weight=1)

        # Red Button
        if self._is_title_type == False:
            self._red_button = tk.Button(self._parent_frame, bg="#F6A5A3",command=lambda name=self._name:self._queue_manager.discard_student(name))
            self._red_button.grid(row=row_index, column=4, sticky="ew")
            self._parent_frame.grid_columnconfigure(4, weight=1)

        # Green Button
        if self._is_title_type == False:
            self._green_button = tk.Button(self._parent_frame, bg="#A1E1AB",command=lambda name=self._name:self._queue_manager.accept_student(name))
            self._green_button.grid(row=row_index, column=5, sticky="ew")
            self._parent_frame.grid_columnconfigure(5, weight=1)

    def GUI_remove_item(self):
        """
        Remove all GUI widgets related to this student 

        Parameters:
            None
        Returns:
            None
        """
        # Index
        try:
            self._index_lab
        except AttributeError:
            while False:
                break
        else:
            self._index_lab.grid_remove()

        
        # Name
        try:
            self._name_lab
        except AttributeError:
            while False:
                break
        else:
            self._name_lab.grid_remove()
        
        # Question Asked
        try:
            self._asked_question_lab
        except AttributeError:
            while False:
                break
        else:
            self._asked_question_lab.grid_remove()

        # Waiting Time
        try:
            self._waiting_time_lab
        except AttributeError:
            while False:
                break
        else:
            self._waiting_time_lab.grid_remove()

        # Red Button
        try:
            self._red_button
        except AttributeError:
            while False:
                break
        else:
            self._red_button.grid_remove()

        # Green Button
        try:
            self._green_button
        except AttributeError:
            while False:
                break
        else:
            self._green_button.grid_remove()
    
        """
        Below functions are the get/set API to access this student features.

        Parameters:
            Related Student attributes
        Returns:
            Related Student attributes
        """
    def get_is_removed(self):
        return self._is_remove

    def set_is_removed(self,is_removed):
        self._is_remove = is_removed

    def set_start_time(self,time):
        self._start_time = time

    def get_start_time(self):
        return self._start_time
        
    def set_waiting_time(self,time):
        self._waiting_time = time

    def get_waiting_time(self):
        return self._waiting_time

    def get_asked_question_count(self):
        return self._asked_question

    def get_name(self):
        return self._name

    def get_index(self):
        return self._index
    
    def update_waiting_time(self):
        """
        Update this student waiting time according to current time stamp

        Parameters:
            None
        Returns:
            None
        """
        self._waiting_time = time.time() - self._start_time
    
    def increase_asked_question_count(self):
        """
        Increase the asked question number by 1

        Parameters:
            None
        Returns:
            None
        """
        self._asked_question = self._asked_question + 1 

class Statistics(object):
    """
    A class to store a group of statistics data and generate related GUI .
    """
    def __init__(self, parent_manager):
        """
        Construct a class to store the statistics informations related to one type of question.

        Parameters:
            parent_manager(AbstractQuestionManager) : The parent manager class to deal with this type of question
        Returns:
            None
        """
        self._parent_frame = parent_manager
        self.wait_time_2_count_map = {}
        self.wait_time_list = []
        self.stu_list = []
        self._general_bg_color_code = "#FFFFFF"

        # GUI objects
        self._index_lab = []
        self._name_lab = []
        self._asked_question_lab = []
        
    def collect_wait_time_info(self,wait_time):
        """
        API to collect the wait time data

        Parameters:
            wait_time(Decimal)
        Returns:
            None
        """
        self.wait_time_list.append(wait_time)

        try:
            cur_count = self.wait_time_2_count_map[round(wait_time,0)]
        except KeyError:
            self.wait_time_2_count_map[round(wait_time,0)] = 1
            return False
        else:
            self.wait_time_2_count_map[round(wait_time,0)] = cur_count + 1

    def set_student_list(self,stu_list):
        """
        API to store the current student list(not only students showed on the list)

        Parameters:
            stu_list(list)
        Returns:
            None
        """
        self.stu_list = sorted(stu_list,key=lambda x:(-x.get_asked_question_count()))
            
    def get_mean_wait_time(self):
        """
        API to get the mean wait time

        Parameters:
            None
        Returns:
            (decimal)
        """
        if len(self.wait_time_list) == 0:
            return 0
        
        result = 0
        i = 0
        for wait_time in self.wait_time_list:
            i = i + 1
            result = result + wait_time

        return result//i

    def get_median_wait_time(self):
        """
        API to get the median wait time

        Parameters:
            None
        Returns:
            (decimal)
        """
        if len(self.wait_time_list) == 0:
            return 0
        
        ordered_list = sorted(self.wait_time_list)

        list_len = len(ordered_list)

        if list_len%2 == 0:
            return (ordered_list[list_len//2 - 1] + ordered_list[list_len//2]) // 2,0

        return round(ordered_list[(list_len - 1)//2],0)
    
    def get_mode_wait_time(self):
        """
        API to get the mode wait time

        Parameters:
            None
        Returns:
            (decimal)
        """
        if len(self.wait_time_2_count_map.items()) == 0:
            return 0
        
        target_wait_time_list = []
        target_count = 0
        for wait_time,count in self.wait_time_2_count_map.items():
            if count > target_count:
                target_count = count
                target_wait_time_list = []
                target_wait_time_list.append(wait_time)
            elif count == target_count:
                target_wait_time_list.append(wait_time)
            
        return target_wait_time_list

    def get_stu_name_by_index(self,index):
        """
        API to get a student name according to the student index from a sorted student list

        Parameters:
            index(int)
        Returns:
            (str)
        """
        return self.stu_list[index].get_name()

    def get_stu_asked_quesiont_count_by_index(self,index):
        """
        API to get the count of asked questions of a student by the student index

        Parameters:
            index(int)
        Returns:
            (int)
        """
        return self.stu_list[index].get_asked_question_count()

    def GUI_generate_layout(self):
        """
        API to generate the GUI layout of statistics .

        Parameters:
            None
        Returns:
            None
        """
        # Student Hero List
        total_row_index = 0
        self._parent_frame.grid_rowconfigure(total_row_index, weight=1)
        self._hero_list_lab = tk.Label(self._parent_frame, text="Top 10 Students asking most questions",bg=self._general_bg_color_code)
        self._hero_list_lab.grid(row=total_row_index, column=0,columnspan = 3, sticky="ew")
        self._parent_frame.grid_columnconfigure(0, weight=1)
        total_row_index = total_row_index + 1
        
        for row_index in range(len(self.stu_list) + 1):
            # Student Hero List Column Title
            # Index
            self._parent_frame.grid_rowconfigure(total_row_index, weight=1)
            if row_index == 0:
                self._index_lab.append(tk.Label(self._parent_frame, text="#",bg=self._general_bg_color_code))
            else:
                self._index = row_index
                self._index_lab.append(tk.Label(self._parent_frame, text=str(row_index),bg=self._general_bg_color_code))
            self._index_lab[row_index].grid(row=total_row_index, column=0, sticky="ew")
            self._parent_frame.grid_columnconfigure(0, weight=1)
        
            # Name
            if row_index == 0:
                self._name_lab.append(tk.Label(self._parent_frame, text="Name",bg=self._general_bg_color_code))
            else:
                self._name_lab.append(tk.Label(self._parent_frame, text=self.get_stu_name_by_index(row_index - 1),bg=self._general_bg_color_code))
            self._name_lab[row_index].grid(row=total_row_index, column=1, sticky="ew")
            self._parent_frame.grid_columnconfigure(1, weight=1)

            # Question Asked
            if row_index == 0:
                self._asked_question_lab.append(tk.Label(self._parent_frame, text="Questions Asked",bg=self._general_bg_color_code))
            else:
                self._asked_question_lab.append(tk.Label(self._parent_frame, text=str(self.get_stu_asked_quesiont_count_by_index(row_index - 1)),bg=self._general_bg_color_code))
            self._asked_question_lab[row_index].grid(row=total_row_index, column=2, sticky="ew")
            self._parent_frame.grid_columnconfigure(2, weight=1)

            total_row_index = total_row_index + 1

        # Show Mean wait time
        self._parent_frame.grid_rowconfigure(total_row_index, weight=1)
        self._mean_wait_time_lab = tk.Label(self._parent_frame, anchor="w" , text="Mean wait time : " + str(self.get_mean_wait_time()),bg=self._general_bg_color_code)
        self._mean_wait_time_lab.grid(row=total_row_index, column=0,columnspan = 3, sticky="ew")
        total_row_index = total_row_index + 1
         
        # Show Median wait time
        self._parent_frame.grid_rowconfigure(total_row_index, weight=1)
        self._median_wait_time_lab = tk.Label(self._parent_frame, anchor="w" , text="Median wait time : " + str(self.get_median_wait_time()),bg=self._general_bg_color_code)
        self._median_wait_time_lab.grid(row=total_row_index, column=0,columnspan = 3, sticky="ew")
        total_row_index = total_row_index + 1
        
        # Show Mode wait time
        self._parent_frame.grid_rowconfigure(total_row_index, weight=1)
        self._mode_wait_time_lab = tk.Label(self._parent_frame, anchor="w" , text="Mode wait time : " + str(self.get_mode_wait_time()),bg=self._general_bg_color_code)
        self._mode_wait_time_lab.grid(row=total_row_index, column=0,columnspan = 3, sticky="ew")
        total_row_index = total_row_index + 1
        
    def GUI_remove_layout(self):
        """
        API to remove the GUI layout of statistics .

        Parameters:
            None
        Returns:
            None
        """
        # Student Hero List
        try:
            self._hero_list_lab
        except AttributeError:
            while False:
                break
        else:
            self._hero_list_lab.grid_remove()
            
        for row_index in list(reversed(range(len(self._index_lab) + 1))):
            # Index
            try:
                self._index_lab[row_index]
            except IndexError:
                while False:
                    break
            else:
                self._index_lab[row_index].grid_remove()
                del self._index_lab[row_index]

            # Name
            try:
                self._name_lab[row_index]
            except IndexError:
                while False:
                    break
            else:
                self._name_lab[row_index].grid_remove()
                del self._name_lab[row_index]
        
            # Question Asked
            try:
                self._asked_question_lab[row_index]
            except IndexError:
                while False:
                    break
            else:
                self._asked_question_lab[row_index].grid_remove()
                del self._asked_question_lab[row_index]

        # Show Mean wait time
        try:
            self._mean_wait_time_lab
        except AttributeError:
            while False:
                break
        else:
            self._mean_wait_time_lab.grid_remove()
        
         
        # Show Median wait time
        try:
            self._median_wait_time_lab
        except AttributeError:
            while False:
                break
        else:
            self._median_wait_time_lab.grid_remove()
        
        # Show Mode wait time
        try:
            self._mode_wait_time_lab
        except AttributeError:
            while False:
                break
        else:
            self._mode_wait_time_lab.grid_remove() 


class GameApp(object) :
    """An application for drawing lines."""
    def __init__(self, master) :
        """Initialise a DrawingApp object, including widget layout."""
        self._master = master
        master.title("Game Application")
        master.minsize(500, 375)

        self._canvas_x_max = 1000
        self._canvas_y_max = 1000
        
        # Column 0 , row 0 ~ 2
        
        self._canvas = tk.Canvas(master, bd=2, relief=tk.SUNKEN,width=self._canvas_x_max, height=self._canvas_y_max)
        self._master.grid_rowconfigure(0, weight=1)

        #Game Button
        self.big_circle_button = tk.Button(master,
                                    command=self.big_button_selected,
                                    text="Big Circle" ,
                                    highlightbackground="red",
                                    highlightcolor="red",
                                    highlightthickness=10)

        self.mid_circle_button = tk.Button(master,
                                    command=self.mid_button_selected,
                                    text="Middle Circle" ,
                                    highlightbackground="red",
                                    highlightcolor="red",
                                    highlightthickness=10)

    
        self.small_circle_button = tk.Button(master,
                                    command=self.small_button_selected,
                                    text="Small Circle" ,
                                    highlightbackground="red",
                                    highlightcolor="red",
                                    highlightthickness=10)

        self.listInformation = Listbox(master, height=5)

        self.circle_2_radius = {"big":50,"mid":30,"small":15}
        self.selected_circle = "big"

        #self._settings = SettingsFrame(master)
        #self._settings.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)

        self._canvas.grid(row=0, column=0, rowspan=3, sticky="ns")
        self._master.grid_rowconfigure(0, weight=1)
        self._master.grid_columnconfigure(0, weight=4)
        
        self.big_circle_button.grid(row=0, column=1, sticky="ns")
        self._master.grid_rowconfigure(0, weight=1)
        self._master.grid_columnconfigure(1, weight=1)
        
        self.mid_circle_button.grid(row=1, column=1, sticky="ns")
        self._master.grid_rowconfigure(1, weight=1)
        self._master.grid_columnconfigure(1, weight=1)
        
        self.small_circle_button.grid(row=2, column=1, sticky="ns")
        self._master.grid_rowconfigure(2, weight=1)
        self._master.grid_columnconfigure(1, weight=1)

        self.listInformation.grid(row = 3, column = 0,columnspan=2,  sticky="ew")

        self.current_node = MobileNode(self)


        # this will hold the position of the user's "first" click, in the form
        # of (x, y); if the user has not clicked, or clicks a second time, this
        # should be None
        self._start = None

        # create a line of all the lines that the user has drawn
        self._lines = []


        self.index_to_cor = {   0:{"x":10,"y":0},
                                1:{"x":0,"y":0},
                                2:{"x":6,"y":5}}
        # Start Monitor Socket
        try:
            self._socket_monitor = SocketMonitor.SocketMonitor(self.current_node,self.listInformation)
        except AttributeError:
            print("Socket Error")

        self._socket_monitor.SendMessage("EVENT_START","")

    def get_cor_x(self,index):
        if not index in self.index_to_cor:
            return
        return self.index_to_cor.get(index).get("x")

    def get_cor_y(self,index):
        if not index in self.index_to_cor:
            return
        return self.index_to_cor.get(index).get("y")

    def get_canvas(self):
        return self._canvas
    
    def get_x_max(self):
        return self._canvas_x_max
    
    def get_y_max(self):
        return self._canvas_y_max
        
    def big_button_selected(self):
        self.selected_circle = "big"

    def mid_button_selected(self):
        self.selected_circle = "mid"

    def small_button_selected(self):
        self.selected_circle = "small"

    def get_current_radius(self):
        return self.circle_2_radius.get(self.selected_circle,30)
        
    def create_circle(self, x, y, r):
        return self._canvas.create_oval(x-r, y-r, x+r, y+r, fill="blue", outline="#DDD", width=4)
        
    def exit(self) :
        """Close the application."""
        self._master.destroy()

    def clear(self):
        """Delete all lines from the application."""
        self._canvas.delete(tk.ALL)
        self._start = None
        self._lines = []

class NodeAbstract(object):
    def __init__(self, parent_manager) :
        self.parent_manager = parent_manager
        self.radius = 20
        self.x_cor = 0
        self.y_cor = 0
        self.mice_shape = None
        self.mac = ""
        self.color = "black"
        self.PC_TO_REAL_LEN_FACTOR = 350
        self.AUG_FACTOR = 100

    def get_canvas(self):
        return self.parent_manager.get_canvas()

    def create_circle(self, x, y, r):
        return self.get_canvas().create_oval(x-r, y-r, x+r, y+r, fill=self.color, outline="#DDD", width=4)

    def redraw(self):
        if self.mice_shape != None:
            self.get_canvas().delete(self.mice_shape)
        self.mice_shape = self.create_circle(self.x_cor, self.y_cor, self.radius)

    def get_x(self):
        return self.x_cor
    def get_y(self):
        return self.y_cor

class StickNode(NodeAbstract):
    def __init__(self, parent_manager,pos_x,pos_y,rssi,distance,index,mac) :
        super().__init__(parent_manager)
        self.x_cor = pos_x * self.AUG_FACTOR
        self.y_cor = pos_y * self.AUG_FACTOR
        self.rssi = rssi
        self.distance = distance
        self.index = index
        self.mac = mac
        print("new node " + "index = " + str(self.index) + " mac = " + self.mac + " x_cor = " + str(self.x_cor) + " y_cor = " + str(self.y_cor))
        self.redraw()
        self.color = "red"

class MobileNode(NodeAbstract):
    def __init__(self, parent_manager) :
        super().__init__(parent_manager)

        self.cur_total_mac = 0
        self.mac_to_stick_node = {}
        self.index_to_stick_node = {}

        self.RSSI_TO_DIS_C = -2.275354938271597
        self.RSSI_TO_DIS_D = 2.612654320987652
        self.color = "green"

        self.mac_block_list = {"70:03:67:9A:8F:9B":1
                                }
    def calculate_RLS_Z(self,index_i,index_k):
        index_k_obj = self.index_to_stick_node[index_k]
        index_i_obj = self.index_to_stick_node[index_i]
        result = np.power(index_i_obj.distance, 2) - np.power(index_k_obj.distance, 2) - np.power(index_i_obj.x_cor,2) - np.power(index_i_obj.y_cor,2) + np.power(index_k_obj.x_cor,2) + np.power(index_k_obj.y_cor,2)
        return result

    def calculate_RLS_Z_ARRAY(self):
        Z01 = self.calculate_RLS_Z(0,1)
        Z02 = self.calculate_RLS_Z(0,2)
        Z12 = self.calculate_RLS_Z(1,2)
        return np.array([Z01,Z02,Z12])

    def calculate_RLS_X_COF(self,index_i,index_k):
        index_k_obj = self.index_to_stick_node[index_k]
        index_i_obj = self.index_to_stick_node[index_i]
        result = 2 * (index_k_obj.x_cor - index_i_obj.x_cor)
        return result

    def calculate_RLS_Y_COF(self,index_i,index_k):
        index_k_obj = self.index_to_stick_node[index_k]
        index_i_obj = self.index_to_stick_node[index_i]
        result = 2 * (index_k_obj.y_cor - index_i_obj.y_cor)
        return result

    def get_RLS_X_COV_ARRAY(self):
        X01 = self.calculate_RLS_X_COF(0,1)
        X02 = self.calculate_RLS_X_COF(0,2)
        X12 = self.calculate_RLS_X_COF(1,2)
        return np.array([X01,X02,X12])

    def get_RLS_Y_COV_ARRAY(self):
        Y01 = self.calculate_RLS_Y_COF(0,1)
        Y02 = self.calculate_RLS_Y_COF(0,2)
        Y12 = self.calculate_RLS_Y_COF(1,2)
        return np.array([Y01,Y02,Y12])

    def get_RLS_MATRIX(self):
        X_COV_ARRAY = self.get_RLS_X_COV_ARRAY()
        Y_COV_ARRAY = self.get_RLS_Y_COV_ARRAY()
        A = np.vstack([X_COV_ARRAY,Y_COV_ARRAY]).T
        return A

    def update_position(self):
        if self.cur_total_mac < 3:
            return
        A = self.get_RLS_MATRIX()
        Z = self.calculate_RLS_Z_ARRAY()
        X_RLS , Y_RLS = np.linalg.lstsq(A,Z)[0]
        self.x_cor = X_RLS * self.AUG_FACTOR / self.PC_TO_REAL_LEN_FACTOR
        self.y_cor = Y_RLS * self.AUG_FACTOR / self.PC_TO_REAL_LEN_FACTOR
        print("update position (x,y) = " + "(" + str(self.x_cor) + "," + str(self.y_cor) + ")")

    def updateByRSSI(self,mac,rssi_int):
        if rssi_int < -45 and self.cur_total_mac < 3:
            print(mac + " not achieve RSSI level(-50) , cur = " + str(rssi_int))
            return
        if self.cur_total_mac == 3 and mac not in  self.mac_to_stick_node:
            print("Too many node")
            return 


        temp_value = (((-1)*self.RSSI_TO_DIS_C - (rssi_int + 7)/10)/self.RSSI_TO_DIS_D)
        new_distance = np.power(10, temp_value)
        
        if mac not in  self.mac_to_stick_node:
            index = self.cur_total_mac

            x_cor = self.parent_manager.get_cor_x(index)
            y_cor = self.parent_manager.get_cor_y(index)
            currentNode = StickNode(self.parent_manager,x_cor,y_cor,rssi_int,new_distance,index,mac) 
            self.mac_to_stick_node[mac] =  currentNode
            self.index_to_stick_node[index] = currentNode

            self.cur_total_mac = self.cur_total_mac + 1
        else:
            currentNode = self.mac_to_stick_node[mac] 

        currentNode.rssi = rssi_int
        currentNode.distance = new_distance

        print("node " + str(currentNode.index) + " new_distance = " + str(currentNode.distance) + " mac = " + currentNode.mac)
        
    def callback_on_socket_update(self,mac,rssi_str):
        self.updateByRSSI(mac,int(rssi_str))
        self.update_position()
        self.redraw()

class GameWin(object) :
    """An application for drawing lines."""
    def __init__(self, master) :
        """Initialise a DrawingApp object, including widget layout."""
        self._master = master
        master.title("Drawing Application")
        master.minsize(500, 375)

        self.label_font = font.Font(family='Question', size=50, weight='bold')
        self._label_x_max = 1000
        self._label_y_max = 1000

        self._label = tk.Label(master, fg="red",bg="green",font=self.label_font,text="WIN",width=self._label_x_max, height=self._label_y_max)

        self._label.pack(side=tk.TOP,fill="both", expand=True)
    def exit(self) :
        """Close the application."""
        self._master.destroy()

        
if __name__ == "__main__":
    main()         
