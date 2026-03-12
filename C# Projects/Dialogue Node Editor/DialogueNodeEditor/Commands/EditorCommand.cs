using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DialogueNodeEditor.Commands
{
    public interface IEditorCommand
    {
        void Execute();
        void Undo();
    }
}
