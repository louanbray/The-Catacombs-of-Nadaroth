#!/usr/bin/env python3
"""<
Visual Chunk Editor for The Catacombs of Nadaroth
Modern GUI for creating and editing game levels - matches chunk_editor copy.py logic
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
from enum import IntEnum
import os

# Constants - EXACT match with chunk_editor copy.py
class Type(IntEnum):
    WALL = 0
    GATE = 1
    SGATE = 2
    PICKABLE = 3
    ENEMY = 4
    LOOTABLE = 5

class Size(IntEnum):
    COLLAPSE = 0
    SPACED = 1

class Sprite(IntEnum):
    WALL = 11201
    ENEMY = 9053
    VGATE = 9608
    UGATE = 9600
    DGATE = 9604
    ENTITY = 0
    KEY = 9919
    A = 65
    B = 66
    G = 71
    O = 79
    S = 83

class UsableItem(IntEnum):
    NOT_USABLE_ITEM = 0
    BASIC_BOW = 1
    ADVANCED_BOW = 2
    SUPER_BOW = 3
    NADINO_BOW = 4
    BRONZE_KEY = 5
    SILVER_KEY = 6
    GOLD_KEY = 7
    NADINO_KEY = 8
    ONION_RING = 9
    STOCKFISH = 10
    SCHOOL_DISHES = 11
    GOLDEN_APPLE = 12
    BOMB = 13

class Entity(IntEnum):
    NOENTITY = 0
    ENEMY_BRONZE_1 = 1
    ENEMY_BRONZE_2 = 2
    ENEMY_SILVER_1 = 3
    ENEMY_SILVER_2 = 4
    ENEMY_GOLD_1 = 5
    ENEMY_GOLD_2 = 6
    ENEMY_NADINO_1 = 7
    ENEMY_NADINO_2 = 8
    BRONZE_CHEST = 9
    SILVER_CHEST = 10
    GOLD_CHEST = 11
    NADINO_CHEST = 12
    STAR_GATE = 13

CHUNK_WIDTH = 127
CHUNK_HEIGHT = 35

# Item definitions - EXACT match with chunk_editor copy.py
items = {
    "V": [Type.GATE, Sprite.VGATE, Size.COLLAPSE, Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
    "N": [Type.GATE, Sprite.UGATE, Size.COLLAPSE, Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
    "S": [Type.GATE, Sprite.DGATE, Size.COLLAPSE, Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
    "P": [Type.SGATE, Sprite.ENTITY, Size.COLLAPSE, Entity.STAR_GATE, UsableItem.NOT_USABLE_ITEM],
    "W": [Type.WALL, Sprite.WALL, Size.COLLAPSE, Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
    "A": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_BRONZE_1, UsableItem.NOT_USABLE_ITEM],
    "B": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_BRONZE_2, UsableItem.NOT_USABLE_ITEM],
    "C": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_SILVER_1, UsableItem.NOT_USABLE_ITEM],
    "D": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_SILVER_2, UsableItem.NOT_USABLE_ITEM],
    "E": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_GOLD_1, UsableItem.NOT_USABLE_ITEM],
    "F": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_GOLD_2, UsableItem.NOT_USABLE_ITEM],
    "G": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_NADINO_1, UsableItem.NOT_USABLE_ITEM],
    "H": [Type.ENEMY, Sprite.ENEMY, Size.COLLAPSE, Entity.ENEMY_NADINO_2, UsableItem.NOT_USABLE_ITEM],
    "0": [Type.LOOTABLE, Sprite.ENTITY, Size.COLLAPSE, Entity.BRONZE_CHEST, UsableItem.NOT_USABLE_ITEM],
    "1": [Type.LOOTABLE, Sprite.ENTITY, Size.COLLAPSE, Entity.SILVER_CHEST, UsableItem.NOT_USABLE_ITEM],
    "2": [Type.LOOTABLE, Sprite.ENTITY, Size.COLLAPSE, Entity.GOLD_CHEST, UsableItem.NOT_USABLE_ITEM],
    "3": [Type.LOOTABLE, Sprite.ENTITY, Size.COLLAPSE, Entity.NADINO_CHEST, UsableItem.NOT_USABLE_ITEM],
}

# Visual properties for UI
ITEM_DISPLAY = {
    "V": {"color": "#8B4513", "display": "V", "name": "Vertical Gate"},
    "N": {"color": "#8B4513", "display": "N", "name": "North Gate"},
    "S": {"color": "#8B4513", "display": "S", "name": "South Gate"},
    "P": {"color": "#FFD700", "display": "P", "name": "Star Gate"},
    "W": {"color": "#696969", "display": "W", "name": "Wall"},
    "A": {"color": "#CD7F32", "display": "A", "name": "Enemy Bronze 1"},
    "B": {"color": "#CD7F32", "display": "B", "name": "Enemy Bronze 2"},
    "C": {"color": "#C0C0C0", "display": "C", "name": "Enemy Silver 1"},
    "D": {"color": "#C0C0C0", "display": "D", "name": "Enemy Silver 2"},
    "E": {"color": "#FFD700", "display": "E", "name": "Enemy Gold 1"},
    "F": {"color": "#FFD700", "display": "F", "name": "Enemy Gold 2"},
    "G": {"color": "#FF0000", "display": "G", "name": "Enemy Nadino 1"},
    "H": {"color": "#FF0000", "display": "H", "name": "Enemy Nadino 2"},
    "0": {"color": "#CD7F32", "display": "0", "name": "Bronze Chest"},
    "1": {"color": "#C0C0C0", "display": "1", "name": "Silver Chest"},
    "2": {"color": "#FFD700", "display": "2", "name": "Gold Chest"},
    "3": {"color": "#FF1493", "display": "3", "name": "Nadino Chest"},
    " ": {"color": "#F0F0F0", "display": " ", "name": "Empty"},
    "*": {"color": "#1a1a1a", "display": "*", "name": "Spacer"},
}


class ChunkEditor(tk.Tk):
    def __init__(self):
        super().__init__()
        
        self.title("Chunk Editor - The Catacombs of Nadaroth")
        self.geometry("1400x900")
        
        # Initialize with empty chunk (spaces and asterisks)
        self.chunk = []
        for y in range(CHUNK_HEIGHT):
            row = []
            for x in range(CHUNK_WIDTH):
                row.append(" " if x % 2 == 0 else "*")
            self.chunk.append(row)
        
        self.selected_item = " "
        self.cell_buttons = {}
        self.is_drawing = False
        self.is_erasing = False
        self.current_file = None
        
        # UI Setup
        self.setup_ui()
        
        # Bind mouse events for painting (left click)
        self.bind("<ButtonPress-1>", self.start_drawing)
        self.bind("<ButtonRelease-1>", self.stop_drawing)
        self.bind("<B1-Motion>", self.on_drag)
        
        # Bind mouse events for erasing (right click)
        self.bind("<ButtonPress-3>", self.start_erasing)
        self.bind("<ButtonRelease-3>", self.stop_erasing)
        self.bind("<B3-Motion>", self.on_erase_drag)
        
    def setup_ui(self):
        """Create the user interface"""
        main_frame = ttk.Frame(self)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.create_toolbar(main_frame)
        
        content_frame = ttk.Frame(main_frame)
        content_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        self.create_palette(content_frame)
        self.create_grid(content_frame)
        self.create_info_panel(content_frame)
        
        self.create_status_bar(main_frame)
        
    def create_toolbar(self, parent):
        """Create top toolbar with file operations"""
        toolbar = ttk.Frame(parent)
        toolbar.pack(fill=tk.X, pady=(0, 5))
        
        ttk.Button(toolbar, text="📁 New", command=self.new_chunk).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="📂 Open", command=self.open_chunk).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="💾 Save", command=self.save_chunk).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="📋 Export", command=self.export_chunk).pack(side=tk.LEFT, padx=2)
        
        ttk.Separator(toolbar, orient=tk.VERTICAL).pack(side=tk.LEFT, padx=10, fill=tk.Y)
        
        ttk.Button(toolbar, text="🗑️ Clear", command=self.clear_chunk).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="🔄 Fill", command=self.fill_chunk).pack(side=tk.LEFT, padx=2)
        
        ttk.Separator(toolbar, orient=tk.VERTICAL).pack(side=tk.LEFT, padx=10, fill=tk.Y)
        
        ttk.Label(toolbar, text="Zoom:").pack(side=tk.LEFT, padx=(10, 2))
        self.zoom_var = tk.IntVar(value=6)
        zoom_scale = ttk.Scale(toolbar, from_=4, to=12, variable=self.zoom_var, 
                              orient=tk.HORIZONTAL, command=self.update_grid_size)
        zoom_scale.pack(side=tk.LEFT, padx=2)
        
    def create_palette(self, parent):
        """Create item palette on the left"""
        palette_frame = ttk.LabelFrame(parent, text="Items Palette", padding=10)
        palette_frame.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 5))
        
        canvas = tk.Canvas(palette_frame, width=200, height=600)
        scrollbar = ttk.Scrollbar(palette_frame, orient=tk.VERTICAL, command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)
        
        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )
        
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Add items to palette
        categories = {
            "Gates": ["N", "S", "V", "P"],
            "Walls": ["W"],
            "Enemies Bronze": ["A", "B"],
            "Enemies Silver": ["C", "D"],
            "Enemies Gold": ["E", "F"],
            "Enemies Nadino": ["G", "H"],
            "Chests": ["0", "1", "2", "3"],
            "Other": [" "]
        }
        
        for category, item_keys in categories.items():
            ttk.Label(scrollable_frame, text=category, font=("Arial", 10, "bold")).pack(pady=(10, 5), anchor=tk.W)
            for item_key in item_keys:
                props = ITEM_DISPLAY[item_key]
                btn = tk.Button(
                    scrollable_frame,
                    text=f"{props['display']} - {props['name']}",
                    bg=props['color'],
                    fg="white" if props['color'] != "#F0F0F0" else "black",
                    width=20,
                    command=lambda k=item_key: self.select_item(k)
                )
                btn.pack(pady=2, fill=tk.X)
                
    def create_grid(self, parent):
        """Create the main editing grid"""
        grid_container = ttk.Frame(parent)
        grid_container.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        canvas_frame = ttk.Frame(grid_container)
        canvas_frame.pack(fill=tk.BOTH, expand=True)
        
        h_scroll = ttk.Scrollbar(canvas_frame, orient=tk.HORIZONTAL)
        v_scroll = ttk.Scrollbar(canvas_frame, orient=tk.VERTICAL)
        
        self.grid_canvas = tk.Canvas(
            canvas_frame,
            xscrollcommand=h_scroll.set,
            yscrollcommand=v_scroll.set,
            bg="#2b2b2b"
        )
        
        h_scroll.config(command=self.grid_canvas.xview)
        v_scroll.config(command=self.grid_canvas.yview)
        
        h_scroll.pack(side=tk.BOTTOM, fill=tk.X)
        v_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.grid_canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        self.grid_frame = ttk.Frame(self.grid_canvas)
        self.grid_canvas.create_window((0, 0), window=self.grid_frame, anchor="nw")
        
        self.create_grid_cells()
        
    def create_grid_cells(self):
        """Create or update grid cells - reuse existing buttons for performance"""
        cell_size = self.zoom_var.get()
        font = ("Courier", max(6, cell_size - 2))
        
        # Items that occupy 3x3 space: chests (0-3), enemies (A-H), and star gate (P)
        large_items = {'0', '1', '2', '3', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'P'}
        
        # If grid doesn't exist, create it from scratch
        if not self.cell_buttons:
            for y in range(CHUNK_HEIGHT):
                grid_col = 0
                for x in range(CHUNK_WIDTH):
                    if x % 2 == 0:  # Only show even columns
                        btn = tk.Button(
                            self.grid_frame,
                            text="",
                            width=1,
                            height=1,
                            font=font,
                            relief=tk.FLAT,
                            borderwidth=0,
                            command=lambda x=x, y=y: self.paint_cell(x, y)
                        )
                        btn.grid(row=y, column=grid_col, padx=0, pady=0, sticky='nsew')
                        btn.bind("<Enter>", lambda e, x=x, y=y: self.on_cell_enter(x, y))
                        self.cell_buttons[(x, y)] = btn
                        grid_col += 1
        
        # Update all existing buttons with current chunk data
        for y in range(CHUNK_HEIGHT):
            for x in range(CHUNK_WIDTH):
                if x % 2 == 0:  # Only show even columns
                    if (x, y) in self.cell_buttons:
                        item_key = self.chunk[y][x]
                        props = ITEM_DISPLAY.get(item_key, ITEM_DISPLAY[" "])
                        btn = self.cell_buttons[(x, y)]
                        
                        # Check if this cell is in a forbidden zone (next to a 3x3 item)
                        is_forbidden = False
                        if item_key not in large_items:  # Only check if it's not a large item itself
                            for dx in [-2, 0, 2]:
                                for dy in [-1, 0, 1]:
                                    check_x = x + dx
                                    check_y = y + dy
                                    if 0 <= check_x < CHUNK_WIDTH and 0 <= check_y < CHUNK_HEIGHT:
                                        if self.chunk[check_y][check_x] in large_items:
                                            is_forbidden = True
                                            break
                                if is_forbidden:
                                    break
                        
                        # Apply color, with special treatment for forbidden zones
                        if is_forbidden:
                            # Darken the color to show it's forbidden
                            forbidden_color = "#CCCCCC"  # Light gray for forbidden zones
                            btn.config(
                                text=props['display'],
                                bg=forbidden_color,
                                fg="#666666",
                                font=font
                            )
                        else:
                            btn.config(
                                text=props['display'],
                                bg=props['color'],
                                fg="white" if props['color'] not in ["#F0F0F0", "#1a1a1a"] else "black",
                                font=font
                            )
        
        # Batch update scrollregion only if grid was just created
        self.grid_frame.update_idletasks()
        self.grid_canvas.configure(scrollregion=self.grid_canvas.bbox("all"))
    
    def update_forbidden_zones(self):
        """Update the visual coloring of forbidden zones without recreating buttons"""
        cell_size = self.zoom_var.get()
        font = ("Courier", max(6, cell_size - 2))
        
        # Items that occupy 3x3 space
        large_items = {'0', '1', '2', '3', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'P'}
        
        # Update all existing buttons
        for y in range(CHUNK_HEIGHT):
            for x in range(CHUNK_WIDTH):
                if x % 2 == 0:  # Only show even columns
                    if (x, y) in self.cell_buttons:
                        item_key = self.chunk[y][x]
                        props = ITEM_DISPLAY.get(item_key, ITEM_DISPLAY[" "])
                        btn = self.cell_buttons[(x, y)]
                        
                        # Check if this cell is in a forbidden zone (next to a 3x3 item)
                        is_forbidden = False
                        if item_key not in large_items:  # Only check if it's not a large item itself
                            for dx in [-2, 0, 2]:
                                for dy in [-1, 0, 1]:
                                    check_x = x + dx
                                    check_y = y + dy
                                    if 0 <= check_x < CHUNK_WIDTH and 0 <= check_y < CHUNK_HEIGHT:
                                        if self.chunk[check_y][check_x] in large_items:
                                            is_forbidden = True
                                            break
                                if is_forbidden:
                                    break
                        
                        # Apply color, with special treatment for forbidden zones
                        if is_forbidden:
                            forbidden_color = "#CCCCCC"  # Light gray for forbidden zones
                            btn.config(
                                bg=forbidden_color,
                                fg="#666666"
                            )
                        else:
                            btn.config(
                                bg=props['color'],
                                fg="white" if props['color'] not in ["#F0F0F0", "#1a1a1a"] else "black"
                            )
    
    def update_forbidden_zones_around(self, center_x, center_y):
        """Update only the forbidden zones around a specific position (optimized)"""
        cell_size = self.zoom_var.get()
        font = ("Courier", max(6, cell_size - 2))
        
        # Items that occupy 3x3 space
        large_items = {'0', '1', '2', '3', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'P'}
        
        # Only update cells that could be affected (3x3 zone + neighbors)
        for dx in [-4, -2, 0, 2, 4]:
            for dy in [-2, -1, 0, 1, 2]:
                x = center_x + dx
                y = center_y + dy
                
                if x % 2 != 0:  # Skip odd columns (they're spacers)
                    continue
                    
                if 0 <= x < CHUNK_WIDTH and 0 <= y < CHUNK_HEIGHT:
                    if (x, y) in self.cell_buttons:
                        item_key = self.chunk[y][x]
                        props = ITEM_DISPLAY.get(item_key, ITEM_DISPLAY[" "])
                        btn = self.cell_buttons[(x, y)]
                        
                        # Check if this cell is in a forbidden zone
                        is_forbidden = False
                        if item_key not in large_items:
                            for ddx in [-2, 0, 2]:
                                for ddy in [-1, 0, 1]:
                                    check_x = x + ddx
                                    check_y = y + ddy
                                    if 0 <= check_x < CHUNK_WIDTH and 0 <= check_y < CHUNK_HEIGHT:
                                        if self.chunk[check_y][check_x] in large_items:
                                            is_forbidden = True
                                            break
                                if is_forbidden:
                                    break
                        
                        # Apply color
                        if is_forbidden:
                            btn.config(
                                bg="#CCCCCC",
                                fg="#666666"
                            )
                        else:
                            btn.config(
                                bg=props['color'],
                                fg="white" if props['color'] not in ["#F0F0F0", "#1a1a1a"] else "black"
                            )
        
    def create_info_panel(self, parent):
        """Create info panel on the right"""
        info_frame = ttk.LabelFrame(parent, text="Information", padding=10)
        info_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=(5, 0))
        
        ttk.Label(info_frame, text="Selected Item:", font=("Arial", 10, "bold")).pack(anchor=tk.W, pady=(0, 5))
        self.selected_label = ttk.Label(info_frame, text="Empty", font=("Arial", 9))
        self.selected_label.pack(anchor=tk.W, pady=(0, 10))
        
        ttk.Label(info_frame, text="Chunk Info:", font=("Arial", 10, "bold")).pack(anchor=tk.W, pady=(10, 5))
        self.info_text = tk.Text(info_frame, width=25, height=20, wrap=tk.WORD, font=("Arial", 9))
        self.info_text.pack(fill=tk.BOTH, expand=True)
        
        ttk.Label(info_frame, text="Quick Help:", font=("Arial", 10, "bold")).pack(anchor=tk.W, pady=(10, 5))
        help_text = (
            "• Click to place items\n"
            "• Drag to paint\n"
            "• Select from palette\n"
            "• Use Fill for patterns\n"
            "• Export for game use"
        )
        ttk.Label(info_frame, text=help_text, font=("Arial", 8), justify=tk.LEFT).pack(anchor=tk.W)
        
        self.update_info()
        
    def create_status_bar(self, parent):
        """Create bottom status bar"""
        status_frame = ttk.Frame(parent)
        status_frame.pack(fill=tk.X, pady=(5, 0))
        
        self.status_label = ttk.Label(status_frame, text="Ready", relief=tk.SUNKEN)
        self.status_label.pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        self.coords_label = ttk.Label(status_frame, text="X: 0, Y: 0", relief=tk.SUNKEN, width=15)
        self.coords_label.pack(side=tk.RIGHT)
        
    def select_item(self, item_key):
        """Select an item from the palette"""
        self.selected_item = item_key
        props = ITEM_DISPLAY[item_key]
        self.selected_label.config(text=f"{props['display']} - {props['name']}")
        self.status_label.config(text=f"Selected: {props['name']}")
        
    def paint_cell(self, x, y):
        """Paint a cell with the selected item"""
        # Items that occupy 3x3 space: chests (0-3), enemies (A-H), and star gate (P)
        # But the grid is displayed in pairs, so the 3x3 is really at even columns only
        large_items = {'0', '1', '2', '3', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'P'}
        
        # Check if we're placing a large item
        if self.selected_item in large_items:
            # Check borders: large items need space around them (x-2, x, x+2 and y-1, y, y+1)
            # So they can't be placed within 2 units of edges
            if x < 2 or x >= CHUNK_WIDTH - 2 or y < 1 or y >= CHUNK_HEIGHT - 1:
                self.status_label.config(text="Cannot place: too close to edge!")
                return
            
            # Check 3x3 zone: x-2, x, x+2 horizontally (even columns only)
            # and y-1, y, y+1 vertically
            for dx in [-2, 0, 2]:
                for dy in [-1, 0, 1]:
                    if dx == 0 and dy == 0:
                        continue  # Skip center
                    
                    check_x = x + dx
                    check_y = y + dy
                    if 0 <= check_x < CHUNK_WIDTH and 0 <= check_y < CHUNK_HEIGHT:
                        cell = self.chunk[check_y][check_x]
                        # Block if there's anything other than space or asterisk
                        if cell != ' ' and cell != '*':
                            self.status_label.config(text="Cannot place: 3×3 item collision!")
                            return
        else:
            # Check if we're placing a small item - make sure we're not adjacent to a 3x3 item
            # UNLESS we're placing directly on the 3x3 item (to replace it)
            current_cell = self.chunk[y][x]
            is_on_large_item = current_cell in large_items
            
            if not is_on_large_item:
                # We're not on a large item, so check if there's one nearby
                for dx in [-2, 0, 2]:
                    for dy in [-1, 0, 1]:
                        check_x = x + dx
                        check_y = y + dy
                        if 0 <= check_x < CHUNK_WIDTH and 0 <= check_y < CHUNK_HEIGHT:
                            cell = self.chunk[check_y][check_x]
                            # If we find a large item nearby, we're in its 3x3 zone - block!
                            if cell in large_items:
                                self.status_label.config(text="Cannot place: too close to 3×3 item!")
                                return
        
        # Place the item
        if self.selected_item in ['N', 'S']:
            # For continuous gates (N, S), place 2 gates instead of 1 + asterisk
            if x + 1 < CHUNK_WIDTH:
                self.chunk[y][x] = self.selected_item
                self.chunk[y][x + 1] = self.selected_item
                # Update both buttons if they exist
                if (x, y) in self.cell_buttons:
                    props = ITEM_DISPLAY[self.selected_item]
                    btn = self.cell_buttons[(x, y)]
                    btn.config(
                        text=props['display'],
                        bg=props['color'],
                        fg="white" if props['color'] not in ["#F0F0F0", "#1a1a1a"] else "black"
                    )
        else:
            # For other items, place item and asterisk
            self.chunk[y][x] = self.selected_item
            if x % 2 == 0 and x + 1 < CHUNK_WIDTH:
                self.chunk[y][x + 1] = '*'
            
            if (x, y) in self.cell_buttons:
                props = ITEM_DISPLAY[self.selected_item]
                btn = self.cell_buttons[(x, y)]
                btn.config(
                    text=props['display'],
                    bg=props['color'],
                    fg="white" if props['color'] not in ["#F0F0F0", "#1a1a1a"] else "black"
                )
        
        # Redraw only the affected area to show updated forbidden zones (optimized)
        self.update_forbidden_zones_around(x, y)
        self.status_label.config(text="Ready")
        self.update_info()
        
    def on_cell_enter(self, x, y):
        """Handle mouse enter on cell"""
        self.coords_label.config(text=f"X: {x}, Y: {y}")
        if self.is_drawing:
            self.paint_cell(x, y)
    
    def on_drag(self, event):
        """Handle drag motion - find which cell we're over"""
        # Find which button/cell is under the mouse cursor
        widget = self.grid_frame.winfo_containing(event.x_root, event.y_root)
        if widget and hasattr(widget, "master"):
            # Try to find which cell this button belongs to
            for (x, y), btn in self.cell_buttons.items():
                if btn == widget:
                    self.paint_cell_throttled(x, y)
                    break
            
    def start_drawing(self, event):
        """Start drawing mode"""
        self.is_drawing = True
        # Trigger paint on the initial click position
        widget = self.grid_frame.winfo_containing(event.x_root, event.y_root)
        if widget and hasattr(widget, "master"):
            for (x, y), btn in self.cell_buttons.items():
                if btn == widget:
                    self.paint_cell_throttled(x, y)
                    break
        
    def stop_drawing(self, event):
        """Stop drawing mode"""
        self.is_drawing = False
    
    def start_erasing(self, event):
        """Start erasing mode (right click)"""
        self.is_erasing = True
        # Trigger erase on the initial click position
        widget = self.grid_frame.winfo_containing(event.x_root, event.y_root)
        if widget and hasattr(widget, "master"):
            for (x, y), btn in self.cell_buttons.items():
                if btn == widget:
                    self.erase_cell(x, y)
                    break
    
    def stop_erasing(self, event):
        """Stop erasing mode"""
        self.is_erasing = False
    
    def on_erase_drag(self, event):
        """Handle erase drag motion - find which cell we're over"""
        widget = self.grid_frame.winfo_containing(event.x_root, event.y_root)
        if widget and hasattr(widget, "master"):
            for (x, y), btn in self.cell_buttons.items():
                if btn == widget:
                    self.erase_cell(x, y)
                    break
    
    def erase_cell(self, x, y):
        """Erase a cell (set to empty space)"""
        # Clear the cell and its spacer
        if x % 2 == 0:  # Even column
            self.chunk[y][x] = " "
            if x + 1 < CHUNK_WIDTH:
                self.chunk[y][x + 1] = "*"
        
        # Update the button display
        if (x, y) in self.cell_buttons:
            btn = self.cell_buttons[(x, y)]
            btn.config(
                text=" ",
                bg="#F0F0F0",
                fg="black"
            )
        
        # Update forbidden zones around this position
        self.update_forbidden_zones_around(x, y)
        self.status_label.config(text="Erased")
        self.update_info()
    
    def paint_cell_throttled(self, x, y):
        """Paint cell without throttling - system is performant enough"""
        self.paint_cell(x, y)
        
    def update_grid_size(self, value=None):
        """Update grid cell size based on zoom"""
        self.create_grid_cells()
        
    def update_info(self):
        """Update the info panel"""
        # Count items efficiently using a single pass
        item_counts = {}
        for row in self.chunk:
            for cell in row:
                if cell not in (" ", "*"):
                    item_counts[cell] = item_counts.get(cell, 0) + 1
        
        # Build info string
        lines = ["Item Count:"]
        for item_key in sorted(item_counts.keys()):
            count = item_counts[item_key]
            props = ITEM_DISPLAY.get(item_key, {"name": "Unknown"})
            lines.append(f"  {props['name']}: {count}")
        
        info = "\n".join(lines)
        self.info_text.delete("1.0", tk.END)
        self.info_text.insert("1.0", info)
        
    def new_chunk(self):
        """Create a new empty chunk"""
        if messagebox.askyesno("New Chunk", "Create a new chunk? Unsaved changes will be lost."):
            self.chunk = []
            for y in range(CHUNK_HEIGHT):
                row = []
                for x in range(CHUNK_WIDTH):
                    row.append(" " if x % 2 == 0 else "*")
                self.chunk.append(row)
            self.current_file = None
            self.create_grid_cells()
            self.update_info()
            self.status_label.config(text="New chunk created")
            
    def clear_chunk(self):
        """Clear the entire chunk"""
        if messagebox.askyesno("Clear Chunk", "Clear the entire chunk?"):
            self.new_chunk()
            
    def fill_chunk(self):
        """Fill chunk with selected item"""
        if messagebox.askyesno("Fill Chunk", f"Fill entire chunk with {ITEM_DISPLAY[self.selected_item]['name']}?"):
            for y in range(CHUNK_HEIGHT):
                for x in range(CHUNK_WIDTH):
                    if x % 2 == 0:
                        self.chunk[y][x] = self.selected_item
                    else:
                        self.chunk[y][x] = '*'
            self.create_grid_cells()
            self.update_info()
            self.status_label.config(text="Chunk filled")
            
    def open_chunk(self):
        """Open a chunk file - supports both raw grid and coordinate formats"""
        filename = filedialog.askopenfilename(
            title="Open Chunk",
            initialdir="assets/chunks",
            filetypes=[("Chunk files", "*.dodjo"), ("All files", "*.*")]
        )
        if filename:
            try:
                self.status_label.config(text="Loading...")
                self.update()
                
                with open(filename, 'r') as f:
                    lines = f.readlines()
                
                # Detect format: if first line has commas, it's coordinate format
                first_line = lines[0].strip() if lines else ""
                
                if ',' in first_line:
                    # Coordinate format (exported chunks)
                    self._load_from_coordinates(lines)
                else:
                    # Raw grid format (127 chars per line)
                    self._load_from_grid(lines)
                
                self.current_file = filename
                self.create_grid_cells()
                self.update_info()
                self.status_label.config(text=f"Opened: {os.path.basename(filename)}")
            except Exception as e:
                messagebox.showerror("Error", f"Could not open file: {e}")
    
    def _load_from_grid(self, lines):
        """Load chunk from raw 127×35 grid format"""
        self.chunk = []
        for y in range(CHUNK_HEIGHT):
            if y < len(lines):
                line = lines[y].rstrip('\n')
                # Ensure exactly CHUNK_WIDTH characters
                if len(line) < CHUNK_WIDTH:
                    line += ' ' * (CHUNK_WIDTH - len(line))
                elif len(line) > CHUNK_WIDTH:
                    line = line[:CHUNK_WIDTH]
                row = list(line)
            else:
                # Pad missing rows with spaces
                row = [' '] * CHUNK_WIDTH
            self.chunk.append(row)
    
    def _load_from_coordinates(self, lines):
        """Load chunk from coordinate format (exported chunks)"""
        # Initialize empty grid with spaces and asterisks
        self.chunk = [
            [(' ' if x % 2 == 0 else '*') for x in range(CHUNK_WIDTH)]
            for y in range(CHUNK_HEIGHT)
        ]
        
        # Pre-build lookup table for faster symbol finding
        symbol_lookup = {}
        for symbol, item_data in items.items():
            key = (item_data[0], item_data[1], item_data[3], item_data[4])
            symbol_lookup[key] = symbol
        
        # Parse coordinates and place items
        for line in lines:
            line = line.strip()
            if not line:
                continue
            
            try:
                parts = line.split(',')
                x_coord = int(parts[0])
                y_coord = int(parts[1])
                item_type = int(parts[2])
                sprite = int(parts[3])
                row_repeat = int(parts[4])
                col_repeat = int(parts[6])
                entity = int(parts[7])
                usable_item = int(parts[8])
                
                # Convert game coordinates to grid coordinates
                grid_x = x_coord + 63
                grid_y = -y_coord + 17
                
                # Find the corresponding item symbol using lookup
                key = (item_type, sprite, entity, usable_item)
                item_symbol = symbol_lookup.get(key)
                
                if item_symbol and 0 <= grid_x < CHUNK_WIDTH and 0 <= grid_y < CHUNK_HEIGHT:
                    # Place the item with row_repeat and col_repeat
                    start_x = grid_x - row_repeat + 1
                    
                    for rx in range(row_repeat):
                        for cy in range(col_repeat):
                            px = start_x + rx
                            py = grid_y + cy
                            if 0 <= px < CHUNK_WIDTH and 0 <= py < CHUNK_HEIGHT:
                                self.chunk[py][px] = item_symbol
            
            except (ValueError, IndexError):
                continue
                
    def save_chunk(self):
        """Save the chunk"""
        if self.current_file:
            self.save_to_file(self.current_file)
        else:
            self.save_as_chunk()
            
    def save_as_chunk(self):
        """Save chunk as new file"""
        filename = filedialog.asksaveasfilename(
            title="Save Chunk",
            initialdir="assets/chunks",
            defaultextension=".dodjo",
            filetypes=[("Chunk files", "*.dodjo"), ("All files", "*.*")]
        )
        if filename:
            self.save_to_file(filename)
            self.current_file = filename
            
    def save_to_file(self, filename):
        """Save chunk to file - all 127 characters per line"""
        try:
            with open(filename, 'w') as f:
                for row in self.chunk:
                    # Write all 127 characters
                    line = ''.join(row)
                    f.write(line + '\n')
            self.status_label.config(text=f"Saved: {os.path.basename(filename)}")
            messagebox.showinfo("Success", "Chunk saved successfully!")
        except Exception as e:
            messagebox.showerror("Error", f"Could not save file: {e}")
            
    def export_chunk(self):
        """Export chunk using EXACT logic from chunk_editor copy.py"""
        output_lines = []
        chunk_copy = [list(row) for row in self.chunk]
        
        for i in range(CHUNK_HEIGHT):
            row_repeat = col_repeat = 1
            row = list(chunk_copy[i])  # Full 127-char row
            
            for j in range(CHUNK_WIDTH):
                cell = row[j]
                # Skip spaces and asterisks and unknown items
                if cell == '*' or cell == ' ' or cell not in items:
                    pass
                # Check for horizontal repetition (row_repeat)
                elif j < CHUNK_WIDTH - 1 - items[cell][2] and cell == row[j + 1 + items[cell][2]]:
                    row_repeat += 1
                else:
                    # Export this cell
                    # Check for vertical repetition (col_repeat)
                    if i < CHUNK_HEIGHT - 1 and row_repeat == 1:
                        for k in range(CHUNK_HEIGHT - 1 - i):
                            col = list(chunk_copy[i + 1 + k])
                            if cell == col[j]:
                                col_repeat += 1
                                col[j] = ' '
                                chunk_copy[i + 1 + k] = col
                            else:
                                break
                    
                    output_lines.append("%d,%d,%d,%d,%d,%d,%d,%d,%d" % (
                        j - 63,
                        -i + 17,
                        items[cell][0],
                        items[cell][1],
                        row_repeat,
                        items[cell][2],
                        col_repeat,
                        items[cell][3],
                        items[cell][4]
                    ))
                    row_repeat = col_repeat = 1
        
        output = "\n".join(output_lines)
        
        try:
            import pyperclip
            pyperclip.copy(output)
            messagebox.showinfo("Exported", "Chunk exported to clipboard!")
        except ImportError:
            export_window = tk.Toplevel(self)
            export_window.title("Export Chunk")
            export_window.geometry("800x500")
            
            text = tk.Text(export_window, wrap=tk.NONE, font=("Courier", 9))
            text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
            text.insert("1.0", output)
            
            ttk.Label(export_window, text="Format: X, Y, TYPE, SPRITE, ROW_REPEAT, SIZE, COL_REPEAT, ENTITY, USABLE_ITEM").pack(pady=2)
            ttk.Button(export_window, text="Close", command=export_window.destroy).pack(pady=5)


if __name__ == "__main__":
    app = ChunkEditor()
    app.mainloop()
