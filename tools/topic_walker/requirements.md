# Generate a pyqt based production grade application for MacOs/ Mint Linux.

## Purpose
An application with UI interface, that runs on MacOS/Linux. This aims as an interface to capture various topics for C++ study.
It needs to capture different prompts with tags specifying the topics that can potentially be covered under the prompt. 
The source of prompt is human input text that the user adds.
Each prompt can have multiple tags. First will be the source tag and subsequent tags will be additional topics that might be connected or qualify the prompt.
e.g.  prompt "generating lookup tables for template metaprogramming" will have tags "constexpr", "lookup table", "template metaprogramming"
The application has a database to store the data for later reference and analytics.
It also provides a view window to give a tabular form. User can select the direction i.e. "by prompt" where first colum is "prompt"  and following column has a list of tags associate. The user can edit/add/delete tags for the prompt. "by tag" where first column is by "tag" and following column has a list of prompts where this tag appears, where list if prompts is not editable. Additionally, the tags/prompts in the second column is clickable where by -- if user clicks on a tag while in "by prompt" view, the view changes to "by tag" for the tag clicked while in "by tag" view, if the user clicks on a prompt, the view changes to "by prompt" view for the prompt clicked. Additionally, the veiw allows for selecting a tag / prompt with a mouse left button down and drag. Also while navigating the view, trail is maintained for the user actions and user can navigate back on the click actions taken for tag and prompt.
The application uses the UI settings as per the UI of the system. e.g. system fonts, color scheme/ skin is utilized where possible.
Also the application supports zoom in and zoom out in response to native zoom in and zoom out funcationlity e.g. in MacOs, cmd+'+' for zoomin and cmd+'-' for zoom out.

## build
Create a readme on steps to package the application for both MacOs and mint Linux.
As part of the build on MacOs, generate the package for MacOS.


## Setup / first time execution
The application comes with an installer that support below functionality
* Requests for directory to store to save database for capturing prompts and related tags. Default value is ~/.cppTopicWalker/
  * database is stored in UserGivenDirectory/database
  * application related settings are store din UserGivenDirectory/settings/...
  * if the user specifies a path outside of user permissions, return an error message dialog that times out in 1 min and returns to the same prompt where directory to write is being requested. Also on the error message dialog a timer is shown for the seconds remaining to time out and user can close the dialog.
* The application installs in the standard setup as required by the native env. e.g.
  * /Applications/cppTopicWalker.app and related file structure in MacOs and related directories

## uninstallation
* The UI allows for a settings view where other settings are visible. i.e. the storeage directory
* The settings view also has a button to uninstall the application where it removes the application directory structure, database and settings
