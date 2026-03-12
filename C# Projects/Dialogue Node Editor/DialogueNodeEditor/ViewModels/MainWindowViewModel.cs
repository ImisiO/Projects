using DialogueNodeEditor.Commands;
using DialogueNodeEditor.Commands.EditorCommands;
using DialogueNodeEditor.Models;
using DialogueNodeEditor.Services;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;

namespace DialogueNodeEditor.ViewModels
{
    public class MainWindowViewModel :ViewModelBase
    {
        #region Init / Deinit

        /// <summary>
        /// Default constructor for MainWindowViewModel object
        /// </summary>
        public MainWindowViewModel()
        {
            History = new UndoRedoService();
            
            // Seed two starter nodes
            AddDialogueNode(80, 120, "Greeting", "Guard");
            AddDialogueNode(460, 120, "PlayerResponse");
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Undo & Redo history</summary>
        public UndoRedoService History { get; }

        /// <summary>List of all dialogue nodes</summary>
        public ObservableCollection<DialogueNodeViewModel> Nodes { get; } = new();
        
        /// <summary>List of all connection between nodes</summary>
        public ObservableCollection<DialogueConnectionViewModel> Connections { get; } = new();

        /// <summary>[STORE] The node that the line is being drawn from before connecting to another node</summary>
        private DialogueNodeViewModel? _pendingFromNode;
        /// <summary>The node that the line is being drawn from before connecting to another node</summary>
        public DialogueNodeViewModel? PendingFromNode
        {
            get => _pendingFromNode;
            private set => SetField(ref _pendingFromNode, value);
        }

        /// <summary>[STORE] The preview line/wire connection end position</summary>
        private Point _previewEnd;
        /// <summary>The preview line/wire connection end position</summary>
        public Point PreviewEnd
        {
            get => _previewEnd;
            set
            {
                SetField(ref _previewEnd, value);
                OnPropertyChanged(nameof(PreviewStartPoint));
                OnPropertyChanged(nameof(PreviewEndPoint));
                OnPropertyChanged(nameof(IsDrawingWire));
            }
        }

        /// <summary>Whether or not the user is attempting to connect two nodes</summary>
        public bool IsDrawingWire => PendingFromNode != null;

        /// <summary>Whether or not any nodes are selected</summary>
        public bool IsAnyNodeSelected => Nodes.Any(n => n.IsSelected);

        /// <summary>The preview line/wire connection start position</summary>
        public Point PreviewStartPoint { get; private set; }

        /// <summary>The preview line/wire connection end position</summary>
        public Point PreviewEndPoint => PreviewEnd;

        /// <summary>Number of nodes</summary>
        private int _nodeCounter;

        /// <summary>[STORE] Path of the currently open graph file (null = unsaved)</summary>
        private string? _currentFilePath;
        /// <summary>Path of the currently open graph file (null = unsaved)</summary>
        public string? CurrentFilePath
        {
            get => _currentFilePath;
            private set
            {
                if (SetField(ref _currentFilePath, value)) 
                {
                    OnPropertyChanged(nameof(TitleText));
                }
            }
        }

        /// <summary>Window title</summary>
        public string TitleText => CurrentFilePath != null
            ? $"Dialogue Node Editor — {System.IO.Path.GetFileName(CurrentFilePath)}"
            : "Dialogue Node Editor — Unsaved";

        #endregion // Member Variables

        #region Events

        /// <summary>Wire/Connection recalculation event</summary>
        public event Action<DialogueConnectionViewModel>? RequestWireRecalculation;

        #endregion // Events

        #region Helper Functions

        #region Node Helpers

        /// <summary>
        /// Adds dialogue node
        /// </summary>
        /// <param name="x">X position of node</param>
        /// <param name="y">Y position of node</param>
        /// <param name="dialogueId">Dialogue Id</param>
        /// <param name="owner">Dialogue owner</param>
        /// <param name="dialogue">Dialogue string</param>
        public void AddDialogueNode(double x = 0, double y = 0, string dialogueId = "New Dialogue ...", string owner = "Owner ...")
        {
            // Cascade new nodes so they don't stack
            if (x == 0 && y == 0)
            {
                x = 80 + (_nodeCounter % 6) * 310.0;
                y = 80 + (_nodeCounter / 6) * 220.0;
            }
            _nodeCounter++;

            DialogueNode model = new DialogueNode
            {
                DialogueId = dialogueId,
                Owner = owner,
                X = x,
                Y = y
            };

            History.Record(new AddNodeCommand(this, new DialogueNodeViewModel(model)));
        }

        /// <summary>
        /// Deletes selected dialogue node
        /// </summary>
        public void DeleteSelectedNodes()
        {
            List<DialogueNodeViewModel> selected = Nodes.Where(n => n.IsSelected).ToList();
            
            if (selected.Count == 0) 
            {
                return;
            }
           
            History.Record(new DeleteNodesCommand(this, selected));
        }

        /// <summary>
        /// Moves the passed node to the passed x and y positions
        /// </summary>
        /// <param name="node">Node to move</param>
        /// <param name="newX">New x position of node</param>
        /// <param name="newY">New y position of node</param>
        public void MoveNode(DialogueNodeViewModel node, double newX, double newY)
        {
            node.X = newX;
            node.Y = newY;
            RefreshConnectionsFor(node);
        }

        /// <summary>Deselects all nodes.</summary>
        public void ClearSelection()
        {
            foreach (DialogueNodeViewModel n in Nodes)
            {
                n.IsSelected = false;
            }
            OnPropertyChanged(nameof(IsAnyNodeSelected));
        }

        /// <summary>
        /// Clears the current selection and selects only the passed node/>.
        /// </summary>
        public void SelectNode(DialogueNodeViewModel node)
        {
            ClearSelection();
            node.IsSelected = true;
            OnPropertyChanged(nameof(IsAnyNodeSelected));
        }

        /// <summary>
        /// Clears the current selection and selects every node whose bounding
        /// rect intersects the passed rectangle/>.
        /// </summary>
        public void SelectNodesInRect(Rect selectionRect)
        {
            ClearSelection();

            foreach (DialogueNodeViewModel node in Nodes)
            {
                Rect nodeBounds = new Rect(node.X, node.Y, 270, 180);
                if (selectionRect.IntersectsWith(nodeBounds))
                { 
                    node.IsSelected = true;
                }
            }
            OnPropertyChanged(nameof(IsAnyNodeSelected));
        }

        #endregion // Node Helpers

        #region Wire / Connection Helpers

        /// <summary>
        /// Begins node connection process, also starts drawing preview wire/connection
        /// </summary>
        /// <param name="fromNode">Node to draw preview wire/connection from</param>
        /// <param name="outConnectorCenter">Out connector center position</param>
        public void StartNodeConnection(DialogueNodeViewModel fromNode, Point outConnectorCenter)
        {
            CancelNodeConnection();
            PendingFromNode = fromNode;
            PreviewStartPoint = outConnectorCenter;
            PreviewEnd = outConnectorCenter;
            OnPropertyChanged(nameof(IsDrawingWire));
            OnPropertyChanged(nameof(PreviewStartPoint));
        }

        /// <summary>
        /// Connects two nodes together and completes connection process
        /// </summary>
        /// <param name="toNode">Node to connect to</param>
        /// <param name="inConnectorCenter">In connector center position</param>
        public void CompleteNodeConnection(DialogueNodeViewModel toNode, Point inConnectorCenter)
        {
            if (PendingFromNode == null) return;

            if (toNode == PendingFromNode ||
                Connections.Any(c => c.FromId == PendingFromNode.Id && c.ToId == toNode.Id))
            {
                CancelNodeConnection();
                return;
            }

            DialogueConnection model = new DialogueConnection
            {
                FromId = PendingFromNode.Id,
                ToId = toNode.Id
            };

            DialogueConnectionViewModel connectionVM = new DialogueConnectionViewModel(model);
            connectionVM.Recalculate(PreviewStartPoint, inConnectorCenter);
            History.Record(new AddConnectionCommand(this, connectionVM));

            CancelNodeConnection();
        }

        /// <summary>
        /// Cancels node connection proceess
        /// </summary>
        public void CancelNodeConnection()
        {
            PendingFromNode = null;
            OnPropertyChanged(nameof(IsDrawingWire));
        }

        /// <summary>
        /// Deletes passed connection
        /// </summary>
        /// <param name="conn">Connection to delete</param>
        public void DeleteConnection(DialogueConnectionViewModel conn)
        {
            History.Record(new DeleteConnectionCommand(this, conn));
        }

        #endregion // Wire / Connection Helpers

        #region Geometry Helpers

        /// <summary>
        /// Refreshes connection points for the passed node
        /// </summary>
        /// <param name="node">Node to refresh</param>
        public void RefreshConnectionsFor(DialogueNodeViewModel node)
        {
            foreach (DialogueConnectionViewModel conn in Connections.Where(c => c.FromId == node.Id || c.ToId == node.Id)) 
            {
                RequestWireRecalculation?.Invoke(conn);
            }
        }

        /// <summary>
        /// Refreshes connection points for all connections
        /// </summary>
        public void RefreshAllConnections()
        {
            foreach (DialogueConnectionViewModel conn in Connections) 
            {
                RequestWireRecalculation?.Invoke(conn);
            }
        }

        #endregion // Geometry Helper

        #region Save / Load Helpers

        /// <summary>
        /// Saves to <see cref="CurrentFilePath"/> if known, otherwise opens a Save As dialog.
        /// </summary>
        /// <returns>Whether or not the node graph has been successfully saved to a file</returns>
        public bool Save()
        {
            if (CurrentFilePath == null) 
            {
                return SaveAs();
            }

            DialogueGraphService.Save(BuildGraph(), CurrentFilePath);
            return true;
        }

        /// <summary>
        /// Always opens a Save As dialog, even if a path is already known.
        /// </summary>
        /// <returns>Whether or not the node graph has been successfully saved to a file</returns>
        public bool SaveAs()
        {
            string? path = DialogueGraphService.SaveAs(BuildGraph(), CurrentFilePath);
            if (path == null) 
            {
                return false;
            }

            CurrentFilePath = path;
            return true;
        }

        /// <summary>
        /// Opens an Open dialog and loads the chosen graph, replacing the current state.
        /// </summary>
        /// <returns>True when a graph was loaded successfully.</returns>
        public bool Open()
        {
            (DialogueGraph? graph, string? path) = DialogueGraphService.Open();
            
            if (graph == null || path == null) 
            {
                return false;
            }

            LoadGraph(graph);
            CurrentFilePath = path;
            return true;
        }

        /// <summary>
        /// Opens a Save dialog and exports the current graph as a tree-shaped JSON file.
        /// </summary>
        public void Export()
        {
            DialogueExportService.Export(BuildGraph(), CurrentFilePath);
        }

        /// <summary>
        /// Builds a <see cref="DialogueGraph"/> snapshot from the current VM state.
        /// </summary>
        /// <returns>Dialogue graph from the current view model state</returns>
        private DialogueGraph BuildGraph()
        {
            return new DialogueGraph
            {
                Nodes = Nodes.Select(n => n.Model).ToList(),
                Connections = Connections.Select(c => c.Model).ToList(),
            };
        }

        /// <summary>
        /// Clears the current canvas and repopulates it from the passed graph
        /// </summary>
        private void LoadGraph(DialogueGraph graph)
        {
            ClearGraph();

            // Rebuild nodes
            foreach (DialogueNode model in graph.Nodes)
            {
                Nodes.Add(new DialogueNodeViewModel(model));
                _nodeCounter++;
            }

            // Rebuild connections
            foreach (DialogueConnection model in graph.Connections)
            {
                // Only add the connection if both endpoints still exist in the graph.
                bool fromExists = Nodes.Any(n => n.Id == model.FromId);
                bool toExists = Nodes.Any(n => n.Id == model.ToId);

                if (fromExists && toExists) 
                {
                    Connections.Add(new DialogueConnectionViewModel(model));
                }
            }
        }

        #endregion // Save / Load Helpers

        #region Graph Helpers

        public void ClearGraph() 
        {
            History.Clear();
            Connections.Clear();
            Nodes.Clear();
            CurrentFilePath = null;
            _nodeCounter = 0;
        }

        #endregion // Graph Helpers

        #endregion // Helper Function
    }
}
