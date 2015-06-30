import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

Window {
    visible: true
    id: main_window

    width: 600
    height: 480

    minimumWidth: width
    minimumHeight: height

    maximumHeight: minimumHeight
    maximumWidth: minimumWidth


   Rectangle {
        id : chess_board
        width: 440
        height: 440

        anchors.margins: 20
        color : "grey"
        anchors.left : parent.left
        anchors.top : parent.top

        Component {
            id : board_part
            Item {
                id : wrapper
                width : chess_field.cellWidth
                height : chess_field.cellHeight

                Rectangle {
                    id : board_piece
                    anchors.fill: parent
                    color : cell_color
                }
                Image {
                    id : chess_piece
                    source: image_path
                    width : chess_field.cellWidth
                    height : chess_field.cellHeight
                    smooth : true
                    cache : false
                    fillMode : Image.PreserveAspectFit
                }
                states : [
                    State {
                        name : "selected"
                        when : chess_field.selected_cell == index
                        PropertyChanges { target : dragged_item; source :  chess_board_model.get(index).image_path;
                                          x : mouse.mouseX - width / 2 + chess_board.x + chess_field.x
                                          y: mouse.mouseY - height / 2 + chess_board.y + chess_field.y
                        }
                        PropertyChanges { target : chess_piece; source :  "" }
                    }
                ]
            }
        }

        GridView {
            id : chess_field
            delegate : board_part
            model : chess_board_model
            height : 380
            width : 380
            readonly property int rows_count: 8
            readonly property int cols_count: 8
            cellHeight : height / rows_count
            cellWidth: width / cols_count

            interactive: false

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.leftMargin: 20 + cellWidth / 2

            property int selected_cell : -1

            MouseArea {
                id : mouse
                anchors.fill : parent
                drag { maximumX: width; maximumY: height }

                property int dest_ind

                onPressed : {
                    chess_field.selected_cell = chess_field.indexAt(mouseX, mouseY)
                }
                onReleased : {
                    dest_ind = chess_field.indexAt(mouseX, mouseY)
                    if(chess_field.selected_cell != -1 && dest_ind != -1 && dest_ind != chess_field.selected_cell) {
                        chess_board_model.make_move(chess_field.selected_cell, dest_ind);
                    }
                    chess_field.selected_cell = -1
                }
            }
        }

        Row {
             height : chess_field.cellHeight / 2
             width : chess_field.cellWidth * chess_field.cols_count;

             anchors { top: chess_field.bottom; left: chess_field.left }
             spacing : 0
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "A" }
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "B" }
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "C" }
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "D" }
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "E" }
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "F" }
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "G" }
             BoardSign { width : chess_field.cellWidth; height : parent.height; cln_name: "H" }
         }

        Column {
             height : chess_field.cellHeight * (chess_field.rows_count + 0.5)
             width : chess_field.cellWidth / 2;

             anchors { top: chess_field.top; right: chess_field.left }
             spacing : 0
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "8" }
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "7" }
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "6" }
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "5" }
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "4" }
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "3" }
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "2" }
             BoardSign { width : parent.width; height : chess_field.cellHeight; cln_name: "1" }
             BoardSign { width : parent.width; height : chess_field.cellHeight/2; cln_name: "" }
         }

   }

   Button {
       id : start_btn
       text : "Start"
       width : 80
       anchors { top: parent.top; left: chess_board.right; margins : 20 }

       function start_new_game(){
           chess_board_model.reset_board();
           main_window.current_screen = 2
       }
       function stop_game(){
           chess_board_model.clean_board();
           main_window.current_screen = 1
       }

       states:[
           State{
               name: "start"
               when : main_window.current_screen == 1 || main_window.current_screen == 3
               PropertyChanges { target : start_btn; text :  "Start" }
               PropertyChanges { target : start_btn; onClicked: start_new_game() }
           },
           State{
               name: "stop"
               when : main_window.current_screen == 2
               PropertyChanges { target : start_btn; text :  "Stop" }
               PropertyChanges { target : start_btn; onClicked: stop_game() }
           }
       ]
   }

   Button {
       text : "Load"
       id : load_btn
       width : 80
       anchors { top: start_btn.bottom; left: chess_board.right; margins : 20 }
       function load_game() {
           file_dialog.open()
       }
       function save_game() {
           //file_dialog.open()
       }
       states:[
           State{
               name: "load"
               when : main_window.current_screen == 1 || main_window.current_screen == 3
               PropertyChanges { target : load_btn; text :  "Load" }
               PropertyChanges { target : load_btn; onClicked: load_game() }
           },
           State{
               name: "save"
               when : main_window.current_screen == 2
               PropertyChanges { target : load_btn; text :  "Save" }
               PropertyChanges { target : load_btn; onClicked: save_game() }
           }
       ]
   }

   Item {
       id : dragged_piece
       //anchors.left: chess_field.left
       Image {
           id : dragged_item
           width : chess_field.cellWidth;
           height : chess_field.cellHeight
           source : ""
       }
       //width : chess_field.cellWidth
       //height : chess_field.cellHeight
   }


   FileDialog {
       id: file_dialog
       title: "Please choose a file"
       onAccepted: {
           console.log("You chose: " + file_dialog.fileUrl)
           chess_board_model.load_file(file_dialog.fileUrl)
           main_window.current_screen = 2
       }
       onRejected: {
           console.log("Canceled")
       }
   }

   property int current_screen: 1
   /*Rectangle {
        id : pause
        anchors.left : chess_board.right
        anchors.top : parent.right
    }*/
}
