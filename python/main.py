import tkinter as tk
import time

AllTimes = []

def KeyDisplay(event):
    StartTime = time.perf_counter()
    label.config(text=f"{event.char}")
    EndTime = time.perf_counter()
    AllTimes.append((EndTime-StartTime)*1e9)

window = tk.Tk()

window.bind("<Key>", KeyDisplay)
window.geometry("750x700")
label = tk.Label(window, text="", font=("Arial", 450))
label.pack(padx=0, pady=0)

window.mainloop()

print(f"The mean of all times collected is : {sum(AllTimes)/len(AllTimes)}")
