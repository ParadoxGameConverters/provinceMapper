# provinceMapper
A tool to create mappings between provinces in different Paradox Interactive games.

## Config (manual for preload or directly GUI)

1.  Source map folder containing provinces.bmp or provinces.png and related definition.csv
2.  Ditto for target map folder
3.  Optional provinces_map.txt or similar with optional contents.
4.  Source token (eu4)
5.  Target token (v2)

Tokens are at minimum 2 letters. If province file is loaded it must contain those tokens.

Having those provided allows for ProvinceMapper start.

## Operation

Note: "Click" means left click. Right click will be noted when needed.

### Setup

#### Image shade

It starts off by default, but any work should be done with shade enabled. In the Provinces window enable it from the menu.
Mapping without shade enabled is insane.

#### Picture orientation

In the Provinces window select orientation that suits your monitor setup.

### Working with links

#### Selecting (activating) a link

1.  Click on the link in Links tab.
2.  Alternatively, click on a linked province to automatically activate the related link.

Activated links are green.

#### Deselecting (deactivating) a link

1.  Right click.

#### Creating links

Positioning:
 
1.  To position the new link at a proper location in the Links tab, activate any existing link or comment you want the new link to go above.

Creating (first is prefered):

1.  Click a province. If it's not already linked and you're not working in an active link, this creates a new link with that province inside.
2.  Alternatively, press F3.
3.  Alternatively, press Ctrl-L

#### Editing links

1.  Activate a link.
2.  Click unmapped provinces to add them to the active link.
3.  Click already mapped provinces in the same link to remove them from the link.

#### Moving Links

1.  Press Num+ to move a link down one row.
2.  Press Num- to move a link up one row.

#### Swapping to another link

1.  See "Selecting (activating) a link"

#### Deleting links

1.  Press Del while a link is active
2.  Alternatively, press Ctrl-D.
3.  Alternatively, removing all provinces from a link before deactivating it will auto-delete the link.

Creating a new link via F3/Ctrl-L and then right clicking will automatically delete the blank link. This is WAD.

### Working with map

#### Map focus on link

1.  Activate a link.
2.  Click on it (again) in the Links tab to center map on its contents.

#### Zooming

1.  Scroll down to zoom out.
2.  Scroll up to zoom in.

### Working with comments

#### Adding a comment

Positioning:
 
1.  Position the comment by clicking a link or comment below the place you with it to go.

Creating:

1.  Press F4
2.  Alternatively, press Ctrl-C.

#### Editing a comment

1.  Click the comment twice and enter new content in edit dialog.

#### Moving comments

1.  Press Num+ to move a comment down one row.
2.  Press Num- to move a comment up one row.

#### Deleting a comment

1.  Click the comment.
2.  Press Del.
3.  Alternatively, press Ctrl-D.

### Working with versions

Note: "Versions" are different sets of links, that usually relate to different game versions.
Note: Do NOT alter old versions while you have newer province maps loaded! Revert game to the version you want to run before editing them!

### Add a new version

1.  In Links menu under "Versions" select "New Version" or "Copy Version" for a copy of currently selected version.
2.  Both options will require you to rename the version.

### Edit version name

1.  Select a version you wish to edit.
2.  In Links menu under "Versions" choose "Rename".

### Organizing versions

1. To move the active version left, press Num /.
2. To move the active version right, press Num *.

When outputting versions, lefternmost version goes first.

### Delete a version

Note: Do NOT delete obsolete versions, even if the converter does not support multiple versions presently.

1.  Select a version.
2.  In Links menu under "Versions" choose "Delete Version".

## Saving your work

1.  In File menu of Links window, select "Save Links"
2.  Alternatively, press F5.

Loaded links file will be overwritten.
If there was no loaded links file, links will be saved as "province_mappings.txt".
