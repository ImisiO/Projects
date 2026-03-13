using DialogueNodeEditor.Commands.EditorCommands;
using DialogueNodeEditor.UserControls;
using DialogueNodeEditor.ViewModels;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;

namespace DialogueNodeEditor.Windows
{
    public partial class MainWindow : Window
    {
        #region Init / Deinit

        /// <summary>
        /// Default constructor for MainWindow object
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();

            MainWindowViewModel vm = new MainWindowViewModel();
            DataContext = vm;

            vm.RequestWireRecalculation += OnRequestWireRecalculation;
            vm.History.AfterUndoRedo = () => Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Loaded, new Action(() => vm.RefreshAllConnections()));
            vm.AfterOpen = () => Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Loaded, new Action(() => vm.RefreshAllConnections()));
            vm.GetCursorPosition = () => Mouse.GetPosition(EditorCanvas);

            Loaded += (_, _) => vm.RefreshAllConnections();
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Main window view model</summary>
        private MainWindowViewModel VM => (MainWindowViewModel)DataContext;

        /// <summary>Dialogue node UI that is being dragged around</summary>
        private DialogueNodeControl? _draggingControl;
       
        /// <summary>Amount the node is going to be moved by</summary>
        private Point _dragLastMouse;

        /// <summary>Context menu position</summary>
        private Point _contextMenuCanvasPos;

        /// <summary>Move nodes command</summary>
        private MoveNodesCommand? _moveCommand;

        /// <summary>Whether or not the user is panning</summary>
        private bool _isPanning;

        /// <summary>Viewport-space mouse position when pan began</summary>
        private Point _panStartMouseViewportPos;

        /// <summary>CanvasPan.X when the pan began</summary>
        private double _panStartX;

        /// <summary>CanvasPan.y when the pan began</summary>
        private double _panStartY;         

        /// <summary>Minimum zoom scale</summary>
        private const double MinScale = 0.15;

        /// <summary>Maximum zoom scale</summary>
        private const double MaxScale = 3.0;

        /// <summary>Amount that will be added or subtracted from the current zoom scale when zooming in or out</summary>
        private const double ZoomFactor = 1.12;

        /// <summary>Whether or not the user is box selecting</summary>
        private bool _isBoxSelecting;

        /// <summary>Box select start position</summary>
        private Point _boxSelectStart;

        #endregion // Member Variables

        #region Mouse Event Handlers

        #region Node Handlers

        /// <summary>
        /// Handles selecting nodes
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void NodeCtrl_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            // Select the node whose DataContext was clicked
            if (sender is not DialogueNodeControl nc || nc.VM is not DialogueNodeViewModel nodeVM)
            {
                return;
            }

            if (!nodeVM.IsSelected) 
            {
                VM.SelectNode(nodeVM);
            }

            e.Handled = true;
        }

        /// <summary>
        /// Handles node dragging
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void NodeCtrl_DragHandleMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is not DialogueNodeControl nc || nc.VM is not DialogueNodeViewModel nodeVM)
            {
                return;
            }

            if (!nodeVM.IsSelected) 
            {
                VM.SelectNode(nodeVM);
            }

            _draggingControl = nc;
            _dragLastMouse = e.GetPosition(EditorCanvas);

            _moveCommand = new MoveNodesCommand(VM, VM.Nodes.Where(n => n.IsSelected));

            nc.CaptureMouse();
            e.Handled = true;
        }

        /// <summary>
        /// Handles node out going connector being clicked
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Event arguments</param>
        private void NodeCtrl_OutConnectorClicked(object sender, EventArgs e)
        {
            if (sender is not DialogueNodeControl nc || nc.VM is not DialogueNodeViewModel nodeVM)
            { 
                return; 
            }
           
            Point center = DialogueNodeControl.GetConnectorCenter(nc.PublicOutConnector, EditorCanvas);
            VM.StartNodeConnection(nodeVM, center);
        }

        /// <summary>
        /// Handles node in coming connector being clicked
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Event arguments</param>
        private void NodeCtrl_InConnectorClicked(object sender, EventArgs e)
        {
            if (sender is not DialogueNodeControl nc || nc.VM is not DialogueNodeViewModel nodeVM) 
            {
                return;
            }

            Point center = DialogueNodeControl.GetConnectorCenter(nc.PublicInConnector, EditorCanvas);
            VM.CompleteNodeConnection(nodeVM, center);
            HidePreviewWire();
        }

        /// <summary>
        /// Fired whenever a node control changes size (e.g. text typed into a dialogue box
        /// grows the node). Refreshes all connections attached to that node so the bezier
        /// endpoints stay locked to the connectors.
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Size changed event arguments</param>
        private void NodeCtrl_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (sender is DialogueNodeControl nc && nc.VM is DialogueNodeViewModel nodeVM) 
            {
                VM.RefreshConnectionsFor(nodeVM);
            }
        }

        #endregion // Node Handlers

        #region Canvas Handlers

        /// <summary>
        /// Handles user click mouse movement on canvas
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Canvas_MouseMove(object sender, MouseEventArgs e)
        {
            Point pos = e.GetPosition(EditorCanvas);

            if (_draggingControl != null && e.LeftButton == MouseButtonState.Pressed)
            {
                double dx = pos.X - _dragLastMouse.X;
                double dy = pos.Y - _dragLastMouse.Y;
                _dragLastMouse = pos;

                foreach (DialogueNodeViewModel node in VM.Nodes.Where(n => n.IsSelected))
                {
                    VM.MoveNode(node,
                        Math.Clamp(node.X + dx, 0, EditorCanvas.Width - _draggingControl.ActualWidth),
                        Math.Clamp(node.Y + dy, 0, EditorCanvas.Height - _draggingControl.ActualHeight));
                }
            }

            if (_isBoxSelecting && e.LeftButton == MouseButtonState.Pressed)
            {
                SetSelectionRect(_boxSelectStart, pos);
            }

            if (VM.IsDrawingWire)
            {
                VM.PreviewEnd = pos;
                UpdatePreviewWire();
            }
        }

        /// <summary>
        /// Handles box select behaviour
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Canvas_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Keyboard.ClearFocus();
            VM.CancelNodeConnection();
            VM.ClearSelection();
            HidePreviewWire();

            // Begin box select
            _isBoxSelecting = true;
            _boxSelectStart = e.GetPosition(EditorCanvas);
            SetSelectionRect(_boxSelectStart, _boxSelectStart);
            SelectionRect.Visibility = Visibility.Visible;

            EditorCanvas.CaptureMouse();

            e.Handled = true;
        }

        /// <summary>
        /// Handles user releasing left mouse button
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Canvas_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (_draggingControl != null)
            {
                if (_moveCommand != null)
                {
                    _moveCommand.CommitEndPositions();
    
                    if (_moveCommand.HasMoved()) 
                    {
                        VM.History.Record(_moveCommand);
                    }
                    _moveCommand = null;
                }

                _draggingControl.ReleaseMouseCapture();
                _draggingControl = null;
            }

            if (_isBoxSelecting)
            {
                EditorCanvas.ReleaseMouseCapture();
                _isBoxSelecting = false;

                Point end = e.GetPosition(EditorCanvas);
                Rect rect = MakeRect(_boxSelectStart, end);

                // Only treat as box-select if the user actually dragged (not just clicked)
                if (rect.Width > 3 || rect.Height > 3) 
                {
                    VM.SelectNodesInRect(rect);
                }

                SelectionRect.Visibility = Visibility.Collapsed;
            }
        }

        /// <summary>
        /// Handles context menu opening
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Context menu event arguments</param>
        private void Canvas_ContextMenuOpening(object sender, ContextMenuEventArgs e)
        {
            Point screenPos = EditorCanvas.PointToScreen(new Point(e.CursorLeft, e.CursorTop));
            _contextMenuCanvasPos = EditorCanvas.PointFromScreen(screenPos);

            //VM.OnAnySelectedChanged();
        }

        /// <summary>
        /// Handles context menu "Add Node" option being clicked
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void ContextMenu_AddNode_Click(object sender, RoutedEventArgs e)
        {
            VM.AddDialogueNode(_contextMenuCanvasPos.X, _contextMenuCanvasPos.Y);
        }

        /// <summary>
        /// Handles context menu "Delete Node" option being clicked
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void ContextMenu_DeleteSelected_Click(object sender, RoutedEventArgs e)
        {
            VM.DeleteSelectedNodes();
        }

        /// <summary>
        /// Handles context menu "Copy Node(s)" option being clicked
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void ContextMenu_CopyNode_Click(object sender, RoutedEventArgs e)
        {
            VM.CopySelectedNodes();
        }

        /// <summary>
        /// Handles context menu "Paste Node(s)" option being clicked
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void ContextMenu_PasteNode_Click(object sender, RoutedEventArgs e)
        {
            VM.PasteNodes();
        }

        #endregion // Canvas Handlers

        #region Menu Handlers

        /// <summary>
        /// Handles user clicking on "File" -> "New" option
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void Menu_New_Click(object sender, RoutedEventArgs e)
        {
            const string message ="Are you sure that you would like to clear the current graph?";
            const string caption = "Node Graph New";
            MessageBoxResult result = MessageBox.Show(message, caption, MessageBoxButton.YesNo, MessageBoxImage.Warning);

            if (result == MessageBoxResult.Yes)
            {
                VM.ClearGraph();
            }
        }

        /// <summary>
        /// Handles user clicking on "File" -> "Open" option
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void Menu_Open_Click(object sender, RoutedEventArgs e)
        {
            if (VM.Open())
            {
                Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Loaded, new Action(() => VM.RefreshAllConnections()));
            }
        }

        /// <summary>
        /// Handles user clicking on "File" -> "Save" option
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void Menu_Save_Click(object sender, RoutedEventArgs e)
        {
            VM.Save();
        }

        /// <summary>
        /// Handles user clicking on "File" -> "SaveAs" option
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void Menu_SaveAs_Click(object sender, RoutedEventArgs e)
        {
            VM.SaveAs();
        }

        /// <summary>
        /// Handles user clicking on "File" -> "Export" option
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void Menu_Export_Click(object sender, RoutedEventArgs e)
        {
            VM.Export();
        }

        /// <summary>
        /// Handles user clicking on "File" -> "Exit" option
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void Menu_Exit_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        /// <summary>
        /// Handles user clicking on "Help" option
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Routed event arguments</param>
        private void Menu_Help_Click(object sender, RoutedEventArgs e)
        {
            new ShortcutsWindow { Owner = this }.ShowDialog();
        }

        #endregion // Menu Handlers

        #region Wire/Connection Handlers

        /// <summary>
        /// Handles user right clicking on a connection
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Wire_LeftClick(object sender, MouseButtonEventArgs e)
        {
            if (sender is not Path path) 
            {
                return;
            }

            // Walk up: Path → DataTemplate → ContentPresenter → ItemsControl
            // The DataContext of the Path is the ConnectionViewModel
            if (path.DataContext is DialogueConnectionViewModel conn) 
            {
                VM.DeleteConnection(conn);
            }

            e.Handled = true;
        }

        #endregion // Wire/Connection Handlers

        #region Navigation Handlers

        /// <summary>
        /// Handles viewport panning behaviour
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Viewport_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton != MouseButton.Middle)
            { 
                return;
            }

            _isPanning = true;
            _panStartMouseViewportPos = e.GetPosition(CanvasViewport);
            _panStartX = CanvasPan.X;
            _panStartY = CanvasPan.Y;

            CanvasViewport.CaptureMouse();
            CanvasViewport.Cursor = Cursors.SizeAll;
            e.Handled = true;
        }

        /// <summary>
        /// Handles viewport panning behaviour
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Viewport_MouseMove(object sender, MouseEventArgs e)
        {
            if (!_isPanning)
            { 
                return;
            }

            Point current = e.GetPosition(CanvasViewport);
            CanvasPan.X = _panStartX + (current.X - _panStartMouseViewportPos.X);
            CanvasPan.Y = _panStartY + (current.Y - _panStartMouseViewportPos.Y);
        }

        /// <summary>
        /// Handles viewport panning release/end behaviour 
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Viewport_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton != MouseButton.Middle || !_isPanning) 
            {
                return;
            }

            _isPanning = false;
            CanvasViewport.ReleaseMouseCapture();
            CanvasViewport.Cursor = Cursors.Arrow;
            e.Handled = true;
        }

        /// <summary>
        /// Handles viewport zooming behaviour
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void Viewport_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double factor = e.Delta > 0 ? ZoomFactor : 1.0 / ZoomFactor;
            double newScale = Math.Clamp(CanvasScale.ScaleX * factor, MinScale, MaxScale);

            double realFactor = newScale / CanvasScale.ScaleX;

            Point mouse = e.GetPosition(CanvasViewport);
            CanvasPan.X += (mouse.X - CanvasPan.X) * (1 - realFactor);
            CanvasPan.Y += (mouse.Y - CanvasPan.Y) * (1 - realFactor);

            CanvasScale.ScaleX = newScale;
            CanvasScale.ScaleY = newScale;

            UpdateZoomLabel(newScale);
            e.Handled = true;
        }

        /// <summary>
        /// Handles user clicking on the reset view button
        /// </summary>
        /// <param name="sender">Sender of the event</param>
        /// <param name="e">Mouse button event arguments</param>
        private void ResetViewBtn_Click(object sender, RoutedEventArgs e)
        {
            CanvasScale.ScaleX = 1;
            CanvasScale.ScaleY = 1;
            CanvasPan.X = 0;
            CanvasPan.Y = 0;
            UpdateZoomLabel(1);
        }

        #endregion // Navigation Handlers

        #endregion // Mouse Event Handlers

        #region Key Event Handlers

        /// <summary>
        /// Handles user pressing shortcuts
        /// </summary>
        /// <param name="sender">Sender of event</param>
        /// <param name="e">Key event arguments</param>
        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
        }

        #endregion // Key Event Handlers

        #region Helper Functions

        /// <summary>
        /// Recalculates connection spline points for the passed connection
        /// </summary>
        /// <param name="conn">Connection to recaculate spline points for </param>
        private void OnRequestWireRecalculation(DialogueConnectionViewModel conn)
        {
            DialogueNodeControl? fromCtrl = FindNodeControl(conn.FromId);
            DialogueNodeControl? toCtrl = FindNodeControl(conn.ToId);
            
            if (fromCtrl == null || toCtrl == null) 
            {
                return;
            }

            try
            {
                Point start = DialogueNodeControl.GetConnectorCenter(fromCtrl.PublicOutConnector, EditorCanvas);
                Point end = DialogueNodeControl.GetConnectorCenter(toCtrl.PublicInConnector, EditorCanvas);
                conn.Recalculate(start, end);
            }
            catch 
            {  
                /* node not yet in visual tree */ 
            }
        }

        /// <summary>
        /// Finds dialogue node UI element with passed Guid
        /// </summary>
        /// <param name="nodeId">Node id to check for</param>
        /// <returns>DialogueNodeControl object if there is a DialogueNodeControl with the passed Guid or null</returns>
        private DialogueNodeControl? FindNodeControl(Guid nodeId)
        {
            // Walk the NodesControl's item containers to find the matching NodeControl
            FrameworkElement? itemsPresenter = FindItemsPresenter(NodesControl);
            if (itemsPresenter == null) 
            {
                return null;
            }

            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(itemsPresenter); i++)
            {
                DependencyObject? child = VisualTreeHelper.GetChild(itemsPresenter, i);
                if (FindNodeControlInTree(child, nodeId) is DialogueNodeControl found)
                {
                    return found;
                }
            }
            return null;
        }

        /// <summary>
        /// Finds DialogueNodeControl with passed Guid
        /// </summary>
        /// <param name="root">Tree root</param>
        /// <param name="nodeId">Node id to get node of</param>
        /// <returns>DialogueNodeControl object if there is a DialogueNodeControl with the passed Guid or null</returns>
        private static DialogueNodeControl? FindNodeControlInTree(DependencyObject root, Guid nodeId)
        {
            if (root is DialogueNodeControl nc && nc.VM?.Id == nodeId)
            {
                return nc;
            }

            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++)
            {
                DialogueNodeControl? result = FindNodeControlInTree(VisualTreeHelper.GetChild(root, i), nodeId);
                if (result != null) 
                {
                    return result;
                }
            }
            return null;
        }

        /// <summary>
        /// Finds item's presenter
        /// </summary>
        /// <param name="root">Depency object root</param>
        /// <returns>FrameworkElement object or null</returns>
        private static FrameworkElement? FindItemsPresenter(DependencyObject root)
        {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++)
            {
                DependencyObject? child = VisualTreeHelper.GetChild(root, i);

                if (child is Canvas c && c.IsItemsHost)
                {
                    return c;
                }
                
                FrameworkElement? result = FindItemsPresenter(child);
                
                if (result != null) 
                {
                    return result;
                }
            }
            return null;
        }

        /// <summary>
        /// Updates preview wire
        /// </summary>
        private void UpdatePreviewWire()
        {
            PreviewWire.Visibility = Visibility.Visible;

            Point start = VM.PreviewStartPoint;
            Point end = VM.PreviewEnd;
            double cp = Math.Abs(end.X - start.X) * 0.55 + 60;

            PathFigure figure = new PathFigure { StartPoint = start, IsFilled = false };
            figure.Segments.Add(new BezierSegment(
                new Point(start.X + cp, start.Y),
                new Point(end.X - cp, end.Y),
                end, isStroked: true));

            PathGeometry geo = new PathGeometry();
            geo.Figures.Add(figure);
            PreviewWire.Data = geo;
        }

        /// <summary>
        /// Hides preview wire/connection line
        /// </summary>
        private void HidePreviewWire()
        {
            PreviewWire.Visibility = Visibility.Collapsed;
        }

        /// <summary>
        /// Updates zoom label
        /// </summary>
        /// <param name="scale">New scale value</param>
        private void UpdateZoomLabel(double scale)
        {
            ZoomLabel.Text = $"{scale * 100:F0}%";
        }

        /// <summary>
        /// Positions and sizes the SelectionRect visual to span the two canvas-space corners.
        /// </summary>
        private void SetSelectionRect(Point a, Point b)
        {
            Rect r = MakeRect(a, b);
            Canvas.SetLeft(SelectionRect, r.X);
            Canvas.SetTop(SelectionRect, r.Y);
            SelectionRect.Width = r.Width;
            SelectionRect.Height = r.Height;
        }

        /// <summary>Builds a Rect from two corner points.</summary>
        private static Rect MakeRect(Point a, Point b) => new Rect(Math.Min(a.X, b.X), Math.Min(a.Y, b.Y), Math.Abs(a.X - b.X), Math.Abs(a.Y - b.Y));

        #endregion // Helper Functions
    }
}