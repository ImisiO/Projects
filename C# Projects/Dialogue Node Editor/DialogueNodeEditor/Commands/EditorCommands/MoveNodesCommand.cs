using DialogueNodeEditor.ViewModels;
using System.Windows;

namespace DialogueNodeEditor.Commands.EditorCommands
{
    public sealed class MoveNodesCommand : IEditorCommand
    {
        /// <summary>Main window view model</summary>
        private readonly MainWindowViewModel _vm;

        /// <summary>Nodes to move</summary>
        private readonly List<DialogueNodeViewModel> _nodes;

        /// <summary>Nodes positions before moving</summary>
        private readonly List<Point> _before;
        
        /// <summary>Node positions after moving/summary>
        private List<Point> _after = new();

        /// <summary>Whether or not the node moving is completed or not as</summary>
        public bool IsComplete => _after.Count == _nodes.Count;

        /// <summary>
        /// Constructor for MoveNodesCommand
        /// </summary>
        /// <param name="vm">Main window view model to update</param>
        /// <param name="nodes">Nodes to move</param>
        public MoveNodesCommand(MainWindowViewModel vm, IEnumerable<DialogueNodeViewModel> nodes)
        {
            _vm = vm;
            _nodes = nodes.ToList();
            _before = _nodes.Select(n => new Point(n.X, n.Y)).ToList();
        }

        /// <summary>
        /// Populates _after list of points when the node moving has been completed 
        /// </summary>
        public void CommitEndPositions()
        {
            _after = _nodes.Select(n => new Point(n.X, n.Y)).ToList();
        }

        /// <summary>
        /// Returns true if the nodes actually moved.
        /// </summary>
        public bool HasMoved()
        {
            if (!IsComplete) 
            {
                return false;
            }

            return _nodes.Indices().Any(i => _before[i] != _after[i]);
        }

        /// <summary>
        /// Moves selected nodes to their end positions
        /// </summary>
        public void Execute()
        {
            for (int i = 0; i < _nodes.Count; i++) 
            {
                _vm.MoveNode(_nodes[i], _after[i].X, _after[i].Y);
              
            }
        }

        /// <summary>
        /// Moves nodes back to their original positions
        /// </summary>
        public void Undo()
        {
            for (int i = 0; i < _nodes.Count; i++) 
            {
                _vm.MoveNode(_nodes[i], _before[i].X, _before[i].Y);
            }
        }
    }

    internal static class EnumerableExtensions
    {
        public static IEnumerable<int> Indices<T>(this IList<T> list) =>
            Enumerable.Range(0, list.Count);
    }
}
