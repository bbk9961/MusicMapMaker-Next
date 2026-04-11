#include "logic/BeatmapSyncBuffer.h"

namespace MMM::Logic
{

BeatmapSyncBuffer::BeatmapSyncBuffer()
{
    // 初始化池：分配足够多的缓冲应对高速生产
    for ( int i = 0; i < 10; ++i ) {
        m_storage.push_back(std::make_unique<RenderSnapshot>());
        m_freeQueue.enqueue(m_storage.back().get());
    }

    // 初始化一个安全的读取帧防止刚开始时 nullptr 崩溃
    m_reading = new RenderSnapshot();
    m_storage.push_back(std::unique_ptr<RenderSnapshot>(m_reading));
}

RenderSnapshot* BeatmapSyncBuffer::getWorkingSnapshot()
{
    // 如果没有空闲的缓冲，只能新建一个以保证1对1帧率不断裂
    if ( !m_freeQueue.try_dequeue(m_working) ) {
        m_working = new RenderSnapshot();
        m_storage.push_back(std::unique_ptr<RenderSnapshot>(m_working));
    }
    return m_working;
}

void BeatmapSyncBuffer::pushWorkingSnapshot()
{
    if ( m_working ) {
        m_readyQueue.enqueue(m_working);
        m_working = nullptr;
    }
}

RenderSnapshot* BeatmapSyncBuffer::pullLatestSnapshot()
{
    RenderSnapshot* latest = nullptr;
    if ( m_readyQueue.try_dequeue(latest) ) {
        // 将之前 UI 消费完的缓冲归还到 free 队列
        if ( m_reading ) {
            m_freeQueue.enqueue(m_reading);
        }
        m_reading = latest;
    }
    // 如果 readyQueue 为空，继续复用上一帧的数据 (应对 Logic 降速掉帧情况)
    return m_reading;
}

}  // namespace MMM::Logic